#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "gbi.h"
#include "vector.h"
#include "segment.h"
#include "displaylist.h"

static _Thread_local char dl_errmsg[1024];

const char*
DisplayList_ErrMsg (void)
{
    return dl_errmsg;
}

static void
DisplayList_ErrMsgClr (void)
{
    memset(dl_errmsg, 0, sizeof(dl_errmsg));
}

static int
DisplayList_ErrMsgSet (const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(dl_errmsg, sizeof(dl_errmsg), fmt, ap);
    va_end(ap);

    return -1;
}

static void
DisplayList_ErrMsgStackTrace (segaddr_t segAddr)
{
    char strace[50];

    snprintf(strace, sizeof(strace), "  while processing display list at 0x%08X\n", segAddr);
    strncat(dl_errmsg, strace, sizeof(dl_errmsg) - strlen(dl_errmsg));
}

static int
DisplayList_CopyData (ZObj* obj1, segaddr_t segAddr, size_t size, ZObj* obj2, segaddr_t* newSegAddr, const char* typeName)
{
    void* src = ZObj_FromSegment(obj1, segAddr);
    if (src == NULL)
        return DisplayList_ErrMsgSet("Bad segmented address 0x%08X for object of size 0x%lX\n", segAddr, obj1->limit);

    void* dup = ZObj_SearchDuplicate(obj2, src, size);
    if (dup != NULL)
    {
        // Already exists in the object, point to it
        *newSegAddr = ZObj_ToSegment(obj2, dup);
    }
    else
    {
        // Doesn't already exist in the object, add new
        void* dst = ZObj_Alloc(obj2, size);
        if (dst == NULL)
            return DisplayList_ErrMsgSet("Could not allocate memory for %d bytes for %s copied from %08X\n", size, typeName, segAddr);

        memcpy(dst, src, size);
        *newSegAddr = ZObj_ToSegment(obj2, dst);
    }
    return 0;
}

static int
DisplayList_CopyVtx (ZObj* obj1, segaddr_t segAddr, int n, ZObj* obj2, segaddr_t* newSegAddr)
{
    return DisplayList_CopyData(obj1, segAddr, n * SIZEOF_VTX, obj2, newSegAddr, "Vertices");
}

static int
DisplayList_CopyMtx (ZObj* obj1, segaddr_t segAddr, ZObj* obj2, segaddr_t* newSegAddr)
{
    return DisplayList_CopyData(obj1, segAddr, SIZEOF_MTX, obj2, newSegAddr, "Matrix");
}

static int
DisplayList_CopyMovemem (ZObj* obj1, segaddr_t segAddr, uint32_t len, int idx, ZObj* obj2, segaddr_t* newSegAddr)
{
    const char* typeName = "Movemem";

    switch(idx)
    {
        case G_MV_VIEWPORT:
            typeName = "Viewport";
            break;
        case G_MV_LIGHT:
            typeName = "Light";
            break;
        case G_MV_MATRIX:
            typeName = "Forced Matrix";
            break;
        case G_MV_MMTX:
        case G_MV_PMTX:
        case G_MV_POINT:
        default:
            return DisplayList_ErrMsgSet("Unrecognized Movemem Index %d for data at %08X\n", idx, segAddr);
    }
    return DisplayList_CopyData(obj1, segAddr, len, obj2, newSegAddr, typeName);
}

size_t
DisplayList_Length (ZObj* obj, segaddr_t segAddr)
{
    uint8_t* start = ZObj_FromSegment(obj, segAddr);
    uint8_t* data = start;
    size_t size = obj->limit;
    bool exit = false;
    size_t dlLen = 0;

    if (start == NULL)
        return DisplayList_ErrMsgSet("Bad segmented address %08X\n", segAddr);

    while (!exit && data < start + size)
    {
        size_t cmdlen = SIZEOF_GFX;
        uint32_t w0 = READ_32_BE(data, 0);

        uint8_t cmd = w0 >> 24;

        switch (cmd)
        {
            case G_DL:
                if (SHIFTR(w0, 16, 8) == G_DL_PUSH)
                    break;
                FALLTHROUGH;
            case G_ENDDL:
                exit = true;
                break;

            case G_TEXRECTFLIP:
            case G_TEXRECT:
                // These commands are 128 bits rather than the usual 64 bits
                cmdlen = 16;
                break;

            case G_RDPHALF_2:
            case G_SETOTHERMODE_H:
            case G_SETOTHERMODE_L:
            case G_RDPHALF_1:
            case G_SPNOOP:
            case G_GEOMETRYMODE:
            case G_POPMTX:
            case G_TEXTURE:
            case G_SPECIAL_1:
            case G_SPECIAL_2:
            case G_SPECIAL_3:
            case G_MODIFYVTX:
            case G_CULLDL:
            case G_BRANCH_Z:
            case G_TRI1:
            case G_TRI2:
            case G_QUAD:
            case G_LINE3D:
            case G_NOOP:
            case G_SETCOMBINE:
            case G_SETENVCOLOR:
            case G_SETPRIMCOLOR:
            case G_SETBLENDCOLOR:
            case G_SETFOGCOLOR:
            case G_SETFILLCOLOR:
            case G_FILLRECT:
            case G_RDPSETOTHERMODE:
            case G_SETPRIMDEPTH:
            case G_SETSCISSOR:
            case G_SETCONVERT:
            case G_SETKEYR:
            case G_SETKEYGB:
            case G_RDPFULLSYNC:
            case G_RDPTILESYNC:
            case G_RDPPIPESYNC:
            case G_RDPLOADSYNC:
            case G_MOVEMEM:
            case G_MTX:
            case G_VTX:
            case G_SETTIMG:
            case G_SETTILE:
            case G_LOADTILE:
            case G_LOADBLOCK:
            case G_SETTILESIZE:
            case G_LOADTLUT:
            case G_MOVEWORD:
            case G_DMA_IO:
            case G_LOAD_UCODE:
            case G_SETCIMG:
            case G_SETZIMG:
                break;

            default:
                return DisplayList_ErrMsgSet("Invalid command %02X encountered while determining length of display list at %08X\n", cmd, segAddr);
        }
        dlLen += cmdlen;
        data += cmdlen;
    }
    if (!exit)
        return DisplayList_ErrMsgSet("Hit end of object before finding G_ENDDL\n");

    DisplayList_ErrMsgClr();
    return dlLen;
}

int
DisplayList_Copy (ZObj* obj1, segaddr_t segAddr, ZObj* obj2, segaddr_t* newSegAddr)
{
    // last 8 commands
    int i = 0;
    uint8_t history[8] = { G_NOOP };

#define HISTORY_GET(n) \
    history[(i - 1 - (n) < 0) ? (i - 1 - (n) + ARRLEN(history)) : (i - 1 - (n))]

    // texture engine state tracker
    struct {
        int fmt;
        int siz;
        uint32_t width;
        uint32_t dram;
    } timg = { 0 };

    int lastTimgPos = -1;

    struct {
        // SetTile
        int fmt;
        int siz;
        int line;
        uint16_t tmem;
        int pal;
        int cms;
        int cmt;
        int masks;
        int maskt;
        int shifts;
        int shiftt;
        // SetTileSize
        qu102_t uls;
        qu102_t ult;
        qu102_t lrs;
        qu102_t lrt;
    } tileDescriptors[8] = { 0 };

    // display list
    uint8_t* start;
    uint8_t* data;
    size_t size;
    bool exit;
    int ret;

    Vector dlVec;
    size_t dlLen;

    dlLen = DisplayList_Length(obj1, segAddr);
    if (dlLen == -1)
        return -1;

    Vector_New(&dlVec, SIZEOF_GFX);
    Vector_Reserve(&dlVec, dlLen / SIZEOF_GFX);

    start = ZObj_FromSegment(obj1, segAddr);
    data = start;
    size = obj1->limit;
    exit = false;
    ret = 0;

    while (!exit && data < start + size)
    {
        size_t cmdlen = SIZEOF_GFX;
        uint32_t w0 = READ_32_BE(data, 0);
        uint32_t w1 = READ_32_BE(data, 4);

        int cmd = w0 >> 24;
        switch (cmd)
        {
            /*
             * These commands contain pointers, except G_ENDDL which is here for convenience
             */

            case G_DL:
                // recursively copy called display lists
                if (ZObj_AddressValid(obj1, w1)) {
                    ret = DisplayList_Copy(obj1, w1, obj2, &w1);
                    if (ret != 0)
                    {
                        DisplayList_ErrMsgStackTrace(w1);
                        goto err;
                    }
                }
                // if not branchlist, carry on
                if (SHIFTR(w0, 16, 8) == G_DL_PUSH)
                    break;
                // if branchlist, exit
                FALLTHROUGH;
            case G_ENDDL:
                // exit
                exit = true;
                break;

            case G_MOVEMEM:
                if (ZObj_AddressValid(obj1, w1)) {
                    ret = DisplayList_CopyMovemem(obj1, w1, SHIFTR(w0, 19,  5) * 8 + 1, SHIFTR(w0,  0,  8), obj2, &w1);
                    if (ret != 0)
                        goto err;
                }
                break;

            case G_MTX:
                if (ZObj_AddressValid(obj1, w1)) {
                    ret = DisplayList_CopyMtx(obj1, w1, obj2, &w1);
                    if (ret != 0)
                        goto err;
                }
                break;

            case G_VTX:
                if (ZObj_AddressValid(obj1, w1)) {
                    ret = DisplayList_CopyVtx(obj1, w1, SHIFTR(w0, 12, 8), obj2, &w1);
                    if (ret != 0)
                        goto err;
                }
                break;

            /*
             * Texture and TLUT Loading
             */

            case G_SETTIMG:
                timg.fmt =   SHIFTR(w0, 21, 3);
                timg.siz =   SHIFTR(w0, 19, 2);
                timg.width = SHIFTR(w0, 0, 12) + 1;
                timg.dram =  w1;

                lastTimgPos = dlVec.limit;
                break;

            case G_SETTILE:
                {
                    int tile = SHIFTR(w1, 24, 3);

                    tileDescriptors[tile].fmt =     SHIFTR(w0, 21, 3);
                    tileDescriptors[tile].siz =     SHIFTR(w0, 19, 2);
                    tileDescriptors[tile].line =    SHIFTR(w0,  9, 9);
                    tileDescriptors[tile].tmem =    SHIFTR(w0,  0, 9);
                    tileDescriptors[tile].pal =     SHIFTR(w1, 20, 4);
                    tileDescriptors[tile].cms =     SHIFTR(w1,  8, 2);
                    tileDescriptors[tile].cmt =     SHIFTR(w1, 18, 2);
                    tileDescriptors[tile].masks =   SHIFTR(w1,  4, 4);
                    tileDescriptors[tile].maskt =   SHIFTR(w1, 14, 4);
                    tileDescriptors[tile].shifts =  SHIFTR(w1,  0, 4);
                    tileDescriptors[tile].shiftt =  SHIFTR(w1, 10, 4);
                }
                break;

            case G_LOADTILE:
            case G_LOADBLOCK:
                // These commands are only used inside larger texture macros, so we don't need to do anything special with them
                break;

            case G_SETTILESIZE:
                {
                    int tile = SHIFTR(w1, 24, 3);

                    tileDescriptors[tile].uls =  SHIFTR(w0, 12, 12);
                    tileDescriptors[tile].ult =  SHIFTR(w0,  0, 12);
                    tileDescriptors[tile].lrs =  SHIFTR(w1, 12, 12);
                    tileDescriptors[tile].lrt =  SHIFTR(w1,  0, 12);
                }

                if (HISTORY_GET(0) == G_SETTILE &&
                    HISTORY_GET(1) == G_RDPPIPESYNC &&
                    HISTORY_GET(2) == G_LOADBLOCK &&
                    HISTORY_GET(3) == G_RDPLOADSYNC &&
                    HISTORY_GET(4) == G_SETTILE &&
                    HISTORY_GET(5) == G_SETTIMG)
                {
                    int tile = SHIFTR(w1, 24, 3);
                    // gsDPLoadTextureBlock / gsDPLoadMultiBlock
                    uint32_t addr = timg.dram;
                    int siz = tileDescriptors[tile].siz;
                    uint32_t width = qu102_I(tileDescriptors[tile].lrs) + 1;
                    uint32_t height = qu102_I(tileDescriptors[tile].lrt) + 1;

                    if (ZObj_AddressValid(obj1, addr))
                    {
                        segaddr_t newAddr;
                        size_t size = G_SIZ_BYTES(siz) * width * height;
                        ret = DisplayList_CopyData(obj1, addr, size, obj2, &newAddr, "Texture/Multi Block");
                        if (ret != 0)
                            goto err;

                        void* lastTimg = Vector_At(&dlVec, lastTimgPos);
                        WRITE_32_BE(lastTimg, 4, newAddr);
                    }
                }
                break;

            case G_LOADTLUT:
                if (HISTORY_GET(0) == G_RDPLOADSYNC &&
                    HISTORY_GET(1) == G_SETTILE &&
                    HISTORY_GET(2) == G_RDPTILESYNC &&
                    HISTORY_GET(3) == G_SETTIMG &&
                   (timg.fmt == G_IM_FMT_RGBA || timg.fmt == G_IM_FMT_IA) &&
                    timg.siz == G_IM_SIZ_16b)
                {
                    // gsDPLoadTLUT / gsDPLoadTLUT_pal16 / gsDPLoadTLUT_pal256
                    uint32_t addr = timg.dram;
                    uint32_t count = SHIFTR(w1, 14, 10) + 1;

                    if (ZObj_AddressValid(obj1, addr))
                    {
                        segaddr_t newAddr;
                        size_t size = ALIGN8(G_SIZ_BYTES(G_IM_SIZ_16b) * count);
                        ret = DisplayList_CopyData(obj1, addr, size, obj2, &newAddr, "TLUT");
                        if (ret != 0)
                            goto err;

                        void* lastTimg = Vector_At(&dlVec, lastTimgPos);
                        WRITE_32_BE(lastTimg, 4, newAddr);
                    }
                }
                break;

            /*
             * These commands are 128 bits rather than the usual 64 bits
             */

            case G_TEXRECTFLIP:
            case G_TEXRECT:
                cmdlen = 16;
                break;

            /*
             * These should not appear in objects
             */

            case G_MOVEWORD:
            case G_DMA_IO:
            case G_LOAD_UCODE:
            case G_SETCIMG:
            case G_SETZIMG:
                ret = DisplayList_ErrMsgSet("Unimplemented display list command %02X encountered in %08X\n", cmd, segAddr);
                goto err;

            /*
             * All other commands do not need any special handling, we know they are valid from the length check
             */

            default:
                break;
        }

        // Copy display list command and overwrite w1
        void* written = Vector_PushBack(&dlVec, cmdlen / SIZEOF_GFX, data);
        WRITE_32_BE(written, 4, w1);

        // Increment to next command
        data += cmdlen;

        // Update history ringbuffer
        history[i] = cmd;
        i = (i + 1) % ARRLEN(history);
    }

    // Copy display list to destination zobj
    void* newDl = ZObj_Alloc(obj2, dlLen);
    if (newDl == NULL)
    {
        ret = DisplayList_ErrMsgSet("Could not allocate memory for display list %d bytes long copied from %08X\n", dlLen, segAddr);
        goto err;
    }
    memcpy(newDl, dlVec.start, dlLen);

    *newSegAddr = ZObj_ToSegment(obj2, newDl);
    Vector_Destroy(&dlVec);
    DisplayList_ErrMsgClr();
    return 0;
err:
    *newSegAddr = -1;
    Vector_Destroy(&dlVec);
    return ret;
}

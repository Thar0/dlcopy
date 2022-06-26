#ifndef GBI_H_
#define GBI_H_

#include <stdint.h>

/* rsp commands */
#define G_NOOP              0x00
#define G_RDPHALF_2         0xF1
#define G_SETOTHERMODE_H    0xE3
#define G_SETOTHERMODE_L    0xE2
#define G_RDPHALF_1         0xE1
#define G_SPNOOP            0xE0
#define G_ENDDL             0xDF
#define G_DL                0xDE
#define G_LOAD_UCODE        0xDD
#define G_MOVEMEM           0xDC
#define G_MOVEWORD          0xDB
#define G_MTX               0xDA
#define G_GEOMETRYMODE      0xD9
#define G_POPMTX            0xD8
#define G_TEXTURE           0xD7
#define G_DMA_IO            0xD6
#define G_SPECIAL_1         0xD5
#define G_SPECIAL_2         0xD4
#define G_SPECIAL_3         0xD3
#define G_VTX               0x01
#define G_MODIFYVTX         0x02
#define G_CULLDL            0x03
#define G_BRANCH_Z          0x04
#define G_TRI1              0x05
#define G_TRI2              0x06
#define G_QUAD              0x07
#define G_LINE3D            0x08

/* rdp commands */
#define G_SETCIMG           0xFF
#define G_SETZIMG           0xFE
#define G_SETTIMG           0xFD
#define G_SETCOMBINE        0xFC
#define G_SETENVCOLOR       0xFB
#define G_SETPRIMCOLOR      0xFA
#define G_SETBLENDCOLOR     0xF9
#define G_SETFOGCOLOR       0xF8
#define G_SETFILLCOLOR      0xF7
#define G_FILLRECT          0xF6
#define G_SETTILE           0xF5
#define G_LOADTILE          0xF4
#define G_LOADBLOCK         0xF3
#define G_SETTILESIZE       0xF2
#define G_LOADTLUT          0xF0
#define G_RDPSETOTHERMODE   0xEF
#define G_SETPRIMDEPTH      0xEE
#define G_SETSCISSOR        0xED
#define G_SETCONVERT        0xEC
#define G_SETKEYR           0xEB
#define G_SETKEYGB          0xEA
#define G_RDPFULLSYNC       0xE9
#define G_RDPTILESYNC       0xE8
#define G_RDPPIPESYNC       0xE7
#define G_RDPLOADSYNC       0xE6
#define G_TEXRECTFLIP       0xE5
#define G_TEXRECT           0xE4

/* image formats */
#define G_IM_FMT_RGBA   0
#define G_IM_FMT_YUV    1
#define G_IM_FMT_CI     2
#define G_IM_FMT_IA     3
#define G_IM_FMT_I      4

/* image sizes */
#define G_IM_SIZ_4b     0
#define G_IM_SIZ_8b     1
#define G_IM_SIZ_16b    2
#define G_IM_SIZ_32b    3

#define G_SIZ_BITS(siz)  (4 << (uint32_t)(siz))
#define G_SIZ_BYTES(siz) (G_SIZ_BITS(siz) / 8)

/* movemem indices */
#define G_MV_MMTX        2
#define G_MV_PMTX        6
#define G_MV_VIEWPORT    8
#define G_MV_LIGHT      10
#define G_MV_MATRIX     14
#define G_MV_POINT      12

/* structure sizes */
#define SIZEOF_GFX 8
#define SIZEOF_MTX 0x40
#define SIZEOF_VTX 0x10

/* dl push flag */
#define G_DL_PUSH   0
#define G_DL_NOPUSH 1

/* 10.2 fixed point */
typedef uint16_t qu102_t;

#define qu102_I(x) \
    ((x) >> 2)

#endif

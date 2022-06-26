#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "segment.h"
#include "zobj.h"

NORETURN static void
Fatal (const char *fmt, ...)
{
    va_list args;

    fputs("error: ", stderr);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

static void*
ReadBinFile (const char* filename, size_t* sizeOut)
{
    FILE* file = fopen(filename, "rb");
    uint8_t* buffer = NULL;
    size_t size;

    if (file == NULL)
        Fatal("failed to open file '%s' for reading: %s", filename, strerror(errno));

    // get size
    fseek(file, 0, SEEK_END);
    size = ftell(file);

    // if the file is empty, return NULL buffer and 0 size
    if (size == 0)
        goto end;

    // allocate buffer
    buffer = malloc(size);
    if (buffer == NULL)
        Fatal("could not allocate buffer for file '%s'", filename);

    // read file
    fseek(file, 0, SEEK_SET);
    if (fread(buffer, size, 1, file) != 1)
        Fatal("error reading from file '%s': %s", filename, strerror(errno));

    fclose(file);

end:
    if (sizeOut != NULL)
        *sizeOut = size;
    return buffer;
}

static void
WriteBinFile(const char* filename, const void* data, size_t size)
{
    FILE* file = fopen(filename, "wb");

    if (file == NULL)
        Fatal("failed to open file '%s' for writing: %s", filename, strerror(errno));

    if (fwrite(data, size, 1, file) != 1)
        Fatal("error writing to file '%s': %s", filename, strerror(errno));

    fclose(file);
}

int
ZObj_New (ZObj* zobj, int segNum)
{
    SEGMENT_NUMBER_ASSERT(segNum);

    zobj->buffer = NULL;
    zobj->limit = zobj->capacity = 0;
    zobj->segmentNumber = segNum;
    return 0;
}

int
ZObj_Free (ZObj* zobj)
{
    if (zobj->buffer != NULL)
        free(zobj->buffer);
    zobj->buffer = NULL;
    zobj->limit = zobj->capacity = 0;
    zobj->segmentNumber = 0;
    return 0;
}

int
ZObj_Read (ZObj* zobj, const char* path, int segNum)
{
    SEGMENT_NUMBER_ASSERT(segNum);

    zobj->buffer = ReadBinFile(path, &zobj->limit);
    zobj->capacity = zobj->limit;
    zobj->segmentNumber = segNum;
    return zobj->buffer == NULL;
}

int
ZObj_Write (ZObj* zobj, const char* path)
{
    WriteBinFile(path, zobj->buffer, zobj->limit);
    return 0;
}

void*
ZObj_Alloc (ZObj* zobj, size_t size)
{
    size_t oldSize = zobj->limit;

    zobj->limit += ALIGN8(size);

    if (zobj->buffer == NULL || zobj->limit > zobj->capacity)
    {
        zobj->capacity = zobj->limit * 2;
        zobj->buffer = realloc(zobj->buffer, zobj->capacity);
        memset((uint8_t*)zobj->buffer + oldSize, 0, zobj->limit - oldSize);
    }

    return (uint8_t*)zobj->buffer + oldSize;
}

bool
ZObj_AddressValid (ZObj* zobj, uint32_t segAddr)
{
    return SEGMENT_NUMBER(segAddr) == zobj->segmentNumber;
}

segaddr_t
ZObj_ToSegment (ZObj* zobj, void* ptr)
{
    if (ptr < zobj->buffer || (uint8_t*)ptr >= (uint8_t*)zobj->buffer + zobj->limit)
        return -1;

    size_t offset = (uint8_t*)ptr - (uint8_t*)zobj->buffer;

    return SEGMENT_ADDR(zobj->segmentNumber, offset);
}

void*
ZObj_FromSegment (ZObj* zobj, segaddr_t segAddr)
{
    int segNum = SEGMENT_NUMBER(segAddr);
    uint32_t offset = SEGMENT_OFFSET(segAddr);

    if (segNum != zobj->segmentNumber)
        return NULL;

    return (uint8_t*)zobj->buffer + offset;
}

void*
ZObj_SearchDuplicate (ZObj * zobj, const void* data, size_t size)
{
    const uint8_t* s = data;
    uint8_t* p = zobj->buffer;

    if (data == NULL || zobj->buffer == NULL || size == 0)
        return NULL;

    while (zobj->limit - (p - (uint8_t*)zobj->buffer) >= size)
    {
        if (p[0] == s[0] && memcmp(p, data, size) == 0)
            return p;
        p += 8;
    }

    return NULL;
}

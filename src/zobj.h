#ifndef ZOBJ_H_
#define ZOBJ_H_

#include <stdbool.h>
#include <stdint.h>

#include "segment.h"

typedef struct ZObj {
    void* buffer;
    size_t limit;
    size_t capacity;
    int segmentNumber;
} ZObj;

int
ZObj_New (ZObj* zobj, int segNum);

int
ZObj_Free (ZObj* zobj);

int
ZObj_Read (ZObj* zobj, const char* path, int segNum);

int
ZObj_Write (ZObj* zobj, const char* path);

void*
ZObj_Alloc (ZObj* zobj, size_t size);

bool
ZObj_AddressValid (ZObj* zobj, segaddr_t segAddr);

segaddr_t
ZObj_ToSegment (ZObj* zobj, void* ptr);

void*
ZObj_FromSegment (ZObj* zobj, segaddr_t segAddr);

void*
ZObj_SearchDuplicate (ZObj* zobj, const void* data, size_t size);

#endif

#ifndef DISPLAYLIST_H_
#define DISPLAYLIST_H_

#include "zobj.h"

size_t
DisplayList_Length (ZObj* obj, uint32_t segAddr);

int
DisplayList_Copy (ZObj* obj1, uint32_t segAddr, ZObj* obj2, uint32_t* newSegAddr);

const char*
DisplayList_ErrMsg (void);

#endif

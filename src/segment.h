#ifndef SEGMENT_H_
#define SEGMENT_H_

#include <stdint.h>

typedef uint32_t segaddr_t;

#define NUM_SEGMENTS    16

#define SEGMENT_OFFSET(seg)     ((segaddr_t)(seg) & 0x00FFFFFF)
#define SEGMENT_NUMBER(seg)     (((segaddr_t)(seg) << 4) >> 28)

#define SEGMENT_ADDR(num, off)  ((segaddr_t)(((num) << 24) + (off)))

#ifndef NDEBUG
#include <assert.h>
#define SEGMENT_NUMBER_ASSERT(segNum) \
    assert((segNum) >= 0 && (segNum) < NUM_SEGMENTS);
#else
#define SEGMENT_NUMBER_ASSERT(segNum)
#endif

#endif

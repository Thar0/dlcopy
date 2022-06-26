#ifndef MACROS_H_
#define MACROS_H_

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define NORETURN __attribute__((noreturn))
#define FALLTHROUGH __attribute__((fallthrough))

#define ARRLEN(x) (sizeof(x) / sizeof(*(x)))

// Endianness

#define BSWAP16(x) \
    __builtin_bswap16(x)

#define BSWAP32(x) \
    __builtin_bswap32(x)

#define READ_16_BE(data, offset) \
    BSWAP16(*(uint16_t*)(((uint8_t*)(data)) + (offset)))

#define READ_32_BE(data, offset) \
    BSWAP32(*(uint32_t*)(((uint8_t*)(data)) + (offset)))

#define WRITE_16_BE(data, offset, value) \
    do { (*(uint16_t*)((uint8_t*)(data) + (offset)) = BSWAP16(value)); } while (0)

#define WRITE_32_BE(data, offset, value) \
    do { (*(uint32_t*)((uint8_t*)(data) + (offset)) = BSWAP32(value)); } while (0)

// Bit packing / extraction

#define SHIFTL(v, s, w) \
    ((uint32_t)(((uint32_t)(v) & ((0x01 << (w)) - 1)) << (s)))

#define SHIFTR(v, s, w) \
    ((uint32_t)(((uint32_t)(v) >> (s)) & ((0x01 << (w)) - 1)))

// Align

#define ALIGN8(x) (((x) + 7) & ~7)

#endif

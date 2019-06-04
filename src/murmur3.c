/* MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 * This (mostly) C89 compliant port was written by Ryan Avella, 2019. This
 * file is also placed in the public domain. The author hereby disclaims
 * copyright to this source code.
 *
 * Notes:
 *   The x86 and x64 versions do _not_ produce the same results, as the
 *   algorithms are optimized for their respective platforms. You can still
 *   compile and run any of them on any platform, but your performance with
 *   the non-native version will be less than optimal.
 *
 *   This implementation is C89 compliant with a few notable exceptions. It
 *   requires the exact width integer types uint8_t, uint32_t, and uint64_t
 *   as described in the ISO/IEC 9899:1999 standard, Section 7.18.1.1. The
 *   developer must supply their own stdint.h header if none is provided by
 *   their compiler vendor. */

#include "murmur3.h"

#include <stdint.h>
#include <limits.h>

#define INT_CAN_HOLD_32_BITS  ((((UINT_MAX>>1)+1)>>16)>>15 > 0)
#define INT_CAN_HOLD_64_BITS  ((((((UINT_MAX>>1)+1)>>16)>>16)>>16)>>15 > 0)
#define LONG_CAN_HOLD_64_BITS ((((((ULONG_MAX>>1)+1)>>16)>>16)>>16)>>15 > 0)

#if INT_CAN_HOLD_32_BITS
typedef unsigned int uword32;
#else
typedef uint32_t     uword32;
#endif

#if INT_CAN_HOLD_64_BITS
typedef unsigned int uword64;
#else
typedef uint64_t     uword64;
#endif

/* U64_EXPR(x, y) takes the upper 32 bits and lower 32 bits (expressed in
 * hexadecimal, with the latter not including a leading '0x'), and expands
 * into an unsigned 64-bit integer constant for c99 and greater. For C89/c90,
 * it first tries to use unsigned int/long suffixes if either of these are
 * capable of representing 64-bit types; otherwise, it exapands into an
 * unsigned 64-bit integer constant _expression_, assuming that a 64-bit type
 * exists as a compiler extension and is typedef'd as uint64_t in stdint.h,
 * whether provided by the compiler vendor or by the developer.
 *
 * Example:
 *   U64_EXPR(0x12345678,DEADBEEF) == UINT64_C(0x12345678DEADBEEF) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define U64_EXPR(x, y) UINT64_C(x ## y)
#elif INT_CAN_HOLD_64_BITS
#define U64_EXPR(x, y) x ## y ## u
#elif LONG_CAN_HOLD_64_BITS
#define U64_EXPR(x, y) x ## y ## ul
#else
#define U64_EXPR(x, y) (((uword64)(x ## ul) << 32) + 0x ## y ## ul)
#endif

#if defined(_MIPSEL)           || \
    defined(_M_AMD64)          || \
    defined(_M_ARM )           || \
    defined(_M_IA64)           || \
    defined(_M_IX86)           || \
    defined(_M_X64)            || \
    defined(_WIN32 )           || \
    defined(__AARCH64EL__)     || \
    defined(__ARMEL__)         || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__MIPSEL)          || \
    defined(__MIPSEL__)        || \
    defined(__THUMBEL__)       || \
    defined(__alpha)           || \
    defined(__ia64)            || \
    defined(__i386)            || \
    defined(__itanium__)       || \
    defined(__vax)             || \
    defined(__x86_64)          || \
    defined(__x86_64__)        || \
   (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#define ENDIAN_L_32(x) (x)
#define ENDIAN_L_64(x) (x)

#elif defined(_MIPSEB)        || \
      defined(_M_PPC)         || \
      defined(__AARCH64EB__)  || \
      defined(__ARMEB__)      || \
      defined(__BIG_ENDIAN__) || \
      defined(__MIPSEB)       || \
      defined(__MIPSEB__)     || \
      defined(__THUMBEB__)    || \
     (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

#if (defined(__GNUC__) || defined(__clang__)) && defined(__has_builtin)
#if __has_builtin(__builtin_bswap32)
#define ENDIAN_L_32(x) __builtin_bswap32(x)
#endif
#if __has_builtin(__builtin_bswap64)
#define ENDIAN_L_64(x) __builtin_bswap64(x)
#endif
#endif

#ifndef ENDIAN_L_32
#define ENDIAN_L_32(x)                 \
    (((uword32)(x)<<24)              | \
    (((uword32)(x)<<8) & 0x00ff0000) | \
    (((uword32)(x)>>8) & 0x0000ff00) | \
     ((uword32)(x)>>24))
#endif

#ifndef ENDIAN_L_64
#define ENDIAN_L_64(x)                                     \
    (((uword64)(x)<<56)                                  | \
    (((uword64)(x)<<48) & U64_EXPR(0x00ff0000,00000000)) | \
    (((uword64)(x)<<40) & U64_EXPR(0x0000ff00,00000000)) | \
    (((uword64)(x)<<32) & U64_EXPR(0x000000ff,00000000)) | \
    (((uword64)(x)>>32) & U64_EXPR(0x00000000,ff000000)) | \
    (((uword64)(x)>>40) & U64_EXPR(0x00000000,00ff0000)) | \
    (((uword64)(x)>>48) & U64_EXPR(0x00000000,0000ff00)) | \
     ((uword64)(x)>>56))
#endif

#else
#error "unable to determine endianness"
#endif /* endianness check */

#define getblock32(p, i) ENDIAN_L_32(p[i])
#define getblock64(p, i) ENDIAN_L_64(p[i])

#define ROTL32(x, r) (((uword32)(x) << (r)) | ((uword32)(x) >> (32 - (r))))
#define ROTL64(x, r) (((uword64)(x) << (r)) | ((uword64)(x) >> (64 - (r))))

/* Finalization mix - force all bits of a hash block to avalanche */
#define fmix32(h)         \
    do {                  \
        h ^= h >> 16;     \
        h *= 0x85ebca6bu; \
        h ^= h >> 13;     \
        h *= 0xc2b2ae35u; \
        h ^= h >> 16;     \
    } while(0)

#define fmix64(h)                           \
    do {                                    \
        h ^= h >> 33;                       \
        h *= U64_EXPR(0xff51afd7,ed558ccd); \
        h ^= h >> 33;                       \
        h *= U64_EXPR(0xc4ceb9fe,1a85ec53); \
        h ^= h >> 33;                       \
    } while(0)

void MurmurHash3_x86_32(const void *key, unsigned len, uint32_t seed, uint32_t out[1]) {
    int i;
    const unsigned len_body = len & ~0x03u;
    const unsigned len_tail = len &  0x03;
    const int      nblocks  = len >> 2;

    const uword32 c1 = 0xcc9e2d51;
    const uword32 c2 = 0x1b873593;

    uword32 h1 = seed;

    uword32 k1;

    const uint8_t  *data   = (const uint8_t  *)key;
    const uint32_t *blocks = (const uint32_t *)(data + len_body);
    const uint8_t  *tail   = (const uint8_t  *)(data + len_body);

    /* body */

    for(i = -nblocks; i; i++) {
        k1 = getblock32(blocks, i);

        k1 *= c1;
        k1  = ROTL32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1  = ROTL32(h1, 13);
        h1  = h1*5 + 0xe6546b64;
    }

    /* tail */

    k1 = 0;

    switch(len_tail) {
        case 3:
            k1 = (uword32)tail[2] << 16;
            /* fall-through */
        case 2:
            k1 ^= (uword32)tail[1] << 8;
            /* fall-through */
        case 1:
            k1 ^= (uword32)tail[0];
            k1 *= c1;
            k1  = ROTL32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    }

    /* finalization */

    h1 ^= len;

    fmix32(h1);

    *(uint32_t *)out = (uint32_t)ENDIAN_L_32(h1);
}

void MurmurHash3_x86_128(const void *key, unsigned len, uint32_t seed, uint32_t out[4]) {
    int i;
    const unsigned len_body = len & ~0x0fu;
    const unsigned len_tail = len &  0x0f;
    const int      nblocks  = len >> 4;

    const uword32 c1 = 0x239b961b;
    const uword32 c2 = 0xab0e9789;
    const uword32 c3 = 0x38b34ae5;
    const uword32 c4 = 0xa1e38b93;

    uword32 h1 = seed;
    uword32 h2 = seed;
    uword32 h3 = seed;
    uword32 h4 = seed;

    uword32 k1, k2, k3, k4;

    const uint8_t  *data   = (const uint8_t  *)key;
    const uint32_t *blocks = (const uint32_t *)(data + len_body);
    const uint8_t  *tail   = (const uint8_t  *)(data + len_body);

    /* body */

    for(i = -nblocks; i; i++) {
        k1 = getblock32(blocks, i*4 + 0);
        k2 = getblock32(blocks, i*4 + 1);
        k3 = getblock32(blocks, i*4 + 2);
        k4 = getblock32(blocks, i*4 + 3);

        k1 *= c1;
        k1  = ROTL32(k1, 15);
        k1 *= c2;
        h1 ^= k1;

        h1  = ROTL32(h1, 19);
        h1 += h2;
        h1  = h1*5 + 0x561ccd1b;

        k2 *= c2;
        k2  = ROTL32(k2, 16);
        k2 *= c3;
        h2 ^= k2;

        h2  = ROTL32(h2, 17);
        h2 += h3;
        h2  = h2*5 + 0x0bcaa747;

        k3 *= c3;
        k3  = ROTL32(k3, 17);
        k3 *= c4;
        h3 ^= k3;

        h3  = ROTL32(h3, 15);
        h3 += h4;
        h3  = h3*5 + 0x96cd1c35;

        k4 *= c4;
        k4  = ROTL32(k4, 18);
        k4 *= c1;
        h4 ^= k4;

        h4  = ROTL32(h4, 13);
        h4 += h1;
        h4  = h4*5 + 0x32ac3b17;
    }

    /* tail */

    k1 = k2 = k3 = k4 = 0;

    switch(len_tail) {
        case 15:
            k4 = (uword32)tail[14] << 16;
            /* fall-through */
        case 14:
            k4 ^= (uword32)tail[13] << 8;
            /* fall-through */
        case 13:
            k4 ^= (uword32)tail[12] << 0;
            k4 *= c4;
            k4  = ROTL32(k4, 18);
            k4 *= c1;
            h4 ^= k4;
            /* fall-through */
        case 12:
            k3 = (uword32)tail[11] << 24;
            /* fall-through */
        case 11:
            k3 ^= (uword32)tail[10] << 16;
            /* fall-through */
        case 10:
            k3 ^= (uword32)tail[9] << 8;
            /* fall-through */
        case 9:
            k3 ^= (uword32)tail[8] << 0;
            k3 *= c3;
            k3  = ROTL32(k3, 17);
            k3 *= c4;
            h3 ^= k3;
            /* fall-through */
        case 8:
            k2 = (uword32)tail[7] << 24;
            /* fall-through */
        case 7:
            k2 ^= (uword32)tail[6] << 16;
            /* fall-through */
        case 6:
            k2 ^= (uword32)tail[5] << 8;
            /* fall-through */
        case 5:
            k2 ^= (uword32)tail[4] << 0;
            k2 *= c2;
            k2  = ROTL32(k2, 16);
            k2 *= c3;
            h2 ^= k2;
            /* fall-through */
        case 4:
            k1 = (uword32)tail[3] << 24;
            /* fall-through */
        case 3:
            k1 ^= (uword32)tail[2] << 16;
            /* fall-through */
        case 2:
            k1 ^= (uword32)tail[1] << 8;
            /* fall-through */
        case 1:
            k1 ^= (uword32)tail[0] << 0;
            k1 *= c1;
            k1  = ROTL32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    }

    /* finalization */

    h1 ^= len;
    h2 ^= len;
    h3 ^= len;
    h4 ^= len;

    h1 += h2;
    h1 += h3;
    h1 += h4;
    h2 += h1;
    h3 += h1;
    h4 += h1;

    fmix32(h1);
    fmix32(h2);
    fmix32(h3);
    fmix32(h4);

    h1 += h2;
    h1 += h3;
    h1 += h4;
    h2 += h1;
    h3 += h1;
    h4 += h1;

    ((uint32_t *)out)[0] = (uint32_t)ENDIAN_L_32(h1);
    ((uint32_t *)out)[1] = (uint32_t)ENDIAN_L_32(h2);
    ((uint32_t *)out)[2] = (uint32_t)ENDIAN_L_32(h3);
    ((uint32_t *)out)[3] = (uint32_t)ENDIAN_L_32(h4);
}

void MurmurHash3_x64_128(const void *key, unsigned len, const uint32_t seed, uint64_t out[2]) {
    int i;
    const unsigned len_body = len & ~0x0fu;
    const unsigned len_tail = len &  0x0f;
    const int      nblocks  = len >> 4;

    const uword64 c1 = U64_EXPR(0x87c37b91,114253d5);
    const uword64 c2 = U64_EXPR(0x4cf5ad43,2745937f);

    uword64 h1 = seed;
    uword64 h2 = seed;

    uword64 k1, k2;

    const uint8_t  *data   = (const uint8_t  *)key;
    const uint64_t *blocks = (const uint64_t *)(data);
    const uint8_t  *tail   = (const uint8_t  *)(data + len_body);

    /* body */

    for(i = 0; i < nblocks; i++) {
        k1 = getblock64(blocks, i*2 + 0);
        k2 = getblock64(blocks, i*2 + 1);

        k1 *= c1;
        k1  = ROTL64(k1, 31);
        k1 *= c2;
        h1 ^= k1;

        h1  = ROTL64(h1, 27);
        h1 += h2;
        h1  = h1*5 + 0x52dce729;

        k2 *= c2;
        k2  = ROTL64(k2, 33);
        k2 *= c1;
        h2 ^= k2;

        h2  = ROTL64(h2, 31);
        h2 += h1;
        h2  = h2*5 + 0x38495ab5;
    }

    /* tail */

    k1 = k2 = 0;

    switch(len_tail) {
        case 15:
            k2 = (uword64)tail[14] << 48;
            /* fall-through */
        case 14:
            k2 ^= (uword64)tail[13] << 40;
            /* fall-through */
        case 13:
            k2 ^= (uword64)tail[12] << 32;
            /* fall-through */
        case 12:
            k2 ^= (uword64)tail[11] << 24;
            /* fall-through */
        case 11:
            k2 ^= (uword64)tail[10] << 16;
            /* fall-through */
        case 10:
            k2 ^= (uword64)tail[9] << 8;
            /* fall-through */
        case 9:
            k2 ^= (uword64)tail[8] << 0;
            k2 *= c2;
            k2  = ROTL64(k2, 33);
            k2 *= c1;
            h2 ^= k2;
            /* fall-through */
        case 8:
            k1 = (uword64)tail[7] << 56;
            /* fall-through */
        case 7:
            k1 ^= (uword64)tail[6] << 48;
            /* fall-through */
        case 6:
            k1 ^= (uword64)tail[5] << 40;
            /* fall-through */
        case 5:
            k1 ^= (uword64)tail[4] << 32;
            /* fall-through */
        case 4:
            k1 ^= (uword64)tail[3] << 24;
            /* fall-through */
        case 3:
            k1 ^= (uword64)tail[2] << 16;
            /* fall-through */
        case 2:
            k1 ^= (uword64)tail[1] << 8;
            /* fall-through */
        case 1:
            k1 ^= (uword64)tail[0] << 0;
            k1 *= c1;
            k1  = ROTL64(k1, 31);
            k1 *= c2;
            h1 ^= k1;
    }

    /* finalization */

    h1 ^= len;
    h2 ^= len;

    h1 += h2;
    h2 += h1;

    fmix64(h1);
    fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((uint64_t *)out)[0] = (uint64_t)ENDIAN_L_64(h1);
    ((uint64_t *)out)[1] = (uint64_t)ENDIAN_L_64(h2);
}

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

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "murmur3.h"

#if (SIZE_MAX > UINT32_MAX)
typedef int64_t ssize_type;
#elif (SIZE_MAX > UINT16_MAX)
typedef int32_t ssize_type;
#else
typedef int ssize_type;
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
#define ENDIAN_L_32(x)        \
    (((x)<<24)              | \
    (((x)<<8) & 0x00ff0000) | \
    (((x)>>8) & 0x0000ff00) | \
     ((x)>>24))
#endif

#ifndef ENDIAN_L_64
#define ENDIAN_L_64(x)                 \
    (((x)<<56)                       | \
    (((x)<<48) & 0x00ff000000000000) | \
    (((x)<<40) & 0x0000ff0000000000) | \
    (((x)<<32) & 0x000000ff00000000) | \
    (((x)>>32) & 0x00000000ff000000) | \
    (((x)>>40) & 0x0000000000ff0000) | \
    (((x)>>48) & 0x000000000000ff00) | \
     ((x)>>56))
#endif

#else
#error "unable to determine endianness"
#endif /* endianness check */

#define ROTL32(x, r) (((x) << (r)) | ((x) >> (32 - (r))))
#define ROTL64(x, r) (((x) << (r)) | ((x) >> (64 - (r))))

/* Finalization mix - force all bits of a hash block to avalanche */
#define fmix32(h)        \
    do {                 \
        h ^= h >> 16;    \
        h *= 0x85ebca6b; \
        h ^= h >> 13;    \
        h *= 0xc2b2ae35; \
        h ^= h >> 16;    \
    } while(0)

#define fmix64(h)                \
    do {                         \
        h ^= h >> 33;            \
        h *= 0xff51afd7ed558ccd; \
        h ^= h >> 33;            \
        h *= 0xc4ceb9fe1a85ec53; \
        h ^= h >> 33;            \
    } while(0)

void MurmurHash3_x86_32(const void *key, size_t len, unsigned seed, void *out) {
    ssize_type i;
    const size_t     len_body = len & ~0x03;
    const char       len_tail = len &  0x03;
    const ssize_type nblocks  = len >> 2;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    uint32_t h1 = seed;

    uint32_t k1;

    const uint8_t  *data   = (const uint8_t  *)key;
    const uint32_t *blocks = (const uint32_t *)(data + len_body);
    const uint8_t  *tail   = (const uint8_t  *)(data + len_body);

    /* body */

    for(i = -nblocks; i; i++) {
        k1 = ENDIAN_L_32(blocks[i]);

        k1 *= c1;
        k1  = ROTL32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1  = ROTL32(h1, 13);
        h1  = h1*5 + 0xe6546b64;
    }

    /* tail */

    k1 = 0;

    switch (len_tail) {
        case 3:
            k1  = (uint32_t)tail[2] << 16;
            /* fall-through */
        case 2:
            k1 ^= (uint32_t)tail[1] << 8;
            /* fall-through */
        case 1:
            k1 ^= (uint32_t)tail[0];
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

void MurmurHash3_x86_128(const void *key, size_t len, unsigned seed, void *out) {
    ssize_type i, j;
    const unsigned   len_body = len & ~0x0f;
    const char       len_tail = len &  0x0f;
    const ssize_type nblocks  = len >> 4;

    const uint32_t c1 = 0x239b961b;
    const uint32_t c2 = 0xab0e9789;
    const uint32_t c3 = 0x38b34ae5;
    const uint32_t c4 = 0xa1e38b93;

    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;

    uint32_t k1, k2, k3, k4;

    const uint8_t  *data   = (const uint8_t  *)key;
    const uint32_t *blocks = (const uint32_t *)(data + len_body);
    const uint8_t  *tail   = (const uint8_t  *)(data + len_body);

    /* body */

    for(i = -nblocks; i; i++) {
        j  = i << 2;
        k1 = ENDIAN_L_32(blocks[j + 0]);
        k2 = ENDIAN_L_32(blocks[j + 1]);
        k3 = ENDIAN_L_32(blocks[j + 2]);
        k4 = ENDIAN_L_32(blocks[j + 3]);

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

    switch (len_tail) {
        case 15:
            k4  = (uint32_t)tail[14] << 16;
            /* fall-through */
        case 14:
            k4 ^= (uint32_t)tail[13] << 8;
            /* fall-through */
        case 13:
            k4 ^= (uint32_t)tail[12];
            k4 *= c4;
            k4  = ROTL32(k4, 18);
            k4 *= c1;
            h4 ^= k4;
            /* fall-through */
        case 12:
            k3  = (uint32_t)tail[11] << 24;
            /* fall-through */
        case 11:
            k3 ^= (uint32_t)tail[10] << 16;
            /* fall-through */
        case 10:
            k3 ^= (uint32_t)tail[9] << 8;
            /* fall-through */
        case 9:
            k3 ^= (uint32_t)tail[8];
            k3 *= c3;
            k3  = ROTL32(k3, 17);
            k3 *= c4;
            h3 ^= k3;
            /* fall-through */
        case 8:
            k2  = (uint32_t)tail[7] << 24;
            /* fall-through */
        case 7:
            k2 ^= (uint32_t)tail[6] << 16;
            /* fall-through */
        case 6:
            k2 ^= (uint32_t)tail[5] << 8;
            /* fall-through */
        case 5:
            k2 ^= (uint32_t)tail[4];
            k2 *= c2;
            k2  = ROTL32(k2, 16);
            k2 *= c3;
            h2 ^= k2;
            /* fall-through */
        case 4:
            k1  = (uint32_t)tail[3] << 24;
            /* fall-through */
        case 3:
            k1 ^= (uint32_t)tail[2] << 16;
            /* fall-through */
        case 2:
            k1 ^= (uint32_t)tail[1] << 8;
            /* fall-through */
        case 1:
            k1 ^= (uint32_t)tail[0];
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

void MurmurHash3_x64_128(const void *key, size_t len, unsigned seed, void *out) {
    ssize_type i, j;
    const unsigned   len_body = len & ~0x0f;
    const char       len_tail = len &  0x0f;
    const ssize_type nblocks  = len >> 4;

    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    uint64_t k1, k2;

    const uint8_t  *data   = (const uint8_t  *)key;
    const uint64_t *blocks = (const uint64_t *)(data);
    const uint8_t  *tail   = (const uint8_t  *)(data + len_body);

    /* body */

    for(i = 0; i < nblocks; i++) {
        j   = i << 1;
        k1  = ENDIAN_L_64(blocks[j + 0]);
        k2  = ENDIAN_L_64(blocks[j + 1]);

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

    switch (len_tail) {
        case 15:
            k2  = (uint64_t)tail[14] << 48;
            /* fall-through */
        case 14:
            k2 ^= (uint64_t)tail[13] << 40;
            /* fall-through */
        case 13:
            k2 ^= (uint64_t)tail[12] << 32;
            /* fall-through */
        case 12:
            k2 ^= (uint64_t)tail[11] << 24;
            /* fall-through */
        case 11:
            k2 ^= (uint64_t)tail[10] << 16;
            /* fall-through */
        case 10:
            k2 ^= (uint64_t)tail[9] << 8;
            /* fall-through */
        case 9:
            k2 ^= (uint64_t)tail[8];
            k2 *= c2;
            k2  = ROTL64(k2, 33);
            k2 *= c1;
            h2 ^= k2;
            /* fall-through */
        case 8:
            k1  = (uint64_t)tail[7] << 56;
            /* fall-through */
        case 7:
            k1 ^= (uint64_t)tail[6] << 48;
            /* fall-through */
        case 6:
            k1 ^= (uint64_t)tail[5] << 40;
            /* fall-through */
        case 5:
            k1 ^= (uint64_t)tail[4] << 32;
            /* fall-through */
        case 4:
            k1 ^= (uint64_t)tail[3] << 24;
            /* fall-through */
        case 3:
            k1 ^= (uint64_t)tail[2] << 16;
            /* fall-through */
        case 2:
            k1 ^= (uint64_t)tail[1] << 8;
            /* fall-through */
        case 1:
            k1 ^= (uint64_t)tail[0];
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

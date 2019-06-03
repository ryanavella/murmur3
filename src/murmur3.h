/* MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 * This (mostly) C89 compliant port was written by Ryan Avella, 2019. This
 * file is also placed in the public domain. The author hereby disclaims
 * copyright to this source code. */

#ifndef MURMUR3_H
#define MURMUR3_H

#include <stdint.h>

void MurmurHash3_x86_32 (const void *key, unsigned int len, uint32_t seed, uint32_t out[1]);
void MurmurHash3_x86_128(const void *key, unsigned int len, uint32_t seed, uint32_t out[4]);
void MurmurHash3_x64_128(const void *key, unsigned int len, uint32_t seed, uint64_t out[2]);

#endif /* MURMUR3_H */

/* MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 * This (mostly) C89 compliant port was written by Ryan Avella, 2019. This
 * file is also placed in the public domain. The author hereby disclaims
 * copyright to this source code. */

#ifndef MURMUR3_H
#define MURMUR3_H

void MurmurHash3_x86_32 (const void *key, unsigned len, unsigned seed, void *out);
void MurmurHash3_x86_128(const void *key, unsigned len, unsigned seed, void *out);
void MurmurHash3_x64_128(const void *key, unsigned len, unsigned seed, void *out);

#endif /* MURMUR3_H */

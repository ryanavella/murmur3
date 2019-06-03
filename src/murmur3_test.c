/* Test cases taken from Peter Scott's Murmur3 C port of MurmurHash3, which
 * he has placed in the public domain.
 * See: https://github.com/PeterScott/murmur3
 *
 * This (mostly) C89 compliant port was written by Ryan Avella, 2019. This
 * file is also placed in the public domain. The author hereby disclaims
 * copyright to this source code.
 *
 * TODO: verify these tests work on big-endian machines */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "murmur3.h"

static int passed = 0, failed = 0;

static void hex32(uint32_t *hash, char *buf) {
    sprintf(buf, "%08x", *hash);
}

static void hex128(uint32_t hash[4], char *buf) {
    sprintf(buf, "%08x%08x%08x%08x", hash[0], hash[1], hash[2], hash[3]);
}

static void MurmurHash3_x86_32_test(uint32_t seed, const char *str_in, const char *str_expect) {
    uint32_t hash[1];
    char buf[33];

    MurmurHash3_x86_32(str_in, (unsigned int)strlen(str_in), seed, hash);
    hex32(hash, buf);
    if (strcmp(buf, str_expect) != 0) {
        printf("FAIL(line %i): %s != %s\n", __LINE__, str_in, buf);
        failed++;
        return;
    }
    passed++;
}

static void MurmurHash3_x86_128_test(uint32_t seed, const char *str_in, const char *str_expect) {
    uint32_t hash[4];
    char buf[33];
    MurmurHash3_x86_128(str_in, (unsigned int)strlen(str_in), seed, hash);
    hex128(hash, buf);
    if (strcmp(buf, str_expect) != 0) {
        printf("FAIL(line %i): %s != %s\n", __LINE__, str_in, buf);
        failed++;
        return;
    }
    passed++;
}

static void MurmurHash3_x64_128_test(uint32_t seed, const char *str_in, const char *str_expect) {
    uint64_t hash[2];
    char buf[33];
    MurmurHash3_x64_128(str_in, (unsigned int)strlen(str_in), seed, hash);
    hex128((uint32_t *)hash, buf);
    if (strcmp(buf, str_expect) != 0) {
        printf("FAIL(line %i): %s != %s\n", __LINE__, str_in, buf);
        failed++;
        return;
    }
    passed++;
}

int main(void) {
    MurmurHash3_x86_32_test(1234, "Hello, world!",                "faf6cdb3");
    MurmurHash3_x86_32_test(4321, "Hello, world!",                "bf505788");
    MurmurHash3_x86_32_test(1234, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx", "8905ac28");
    MurmurHash3_x86_32_test(1234, "",                             "0f2cc00b");

    MurmurHash3_x86_128_test(123, "Hello, world!",                "61c9129e5a1aacd7a41621629e37c886");
    MurmurHash3_x86_128_test(321, "Hello, world!",                "d5fbdcb3c26c4193045880c5a7170f0f");
    MurmurHash3_x86_128_test(123, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx", "5e40bab278825a164cf929d31fec6047");
    MurmurHash3_x86_128_test(123, "",                             "fedc524526f3e79926f3e79926f3e799");

    MurmurHash3_x64_128_test(123, "Hello, world!",                "8743acad421c8c73d373c3f5f19732fd");
    MurmurHash3_x64_128_test(321, "Hello, world!",                "f86d4004ca47f42bb9546c7979200aee");
    MurmurHash3_x64_128_test(123, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx", "becf7e04dbcf74637751664ef66e73e0");
    MurmurHash3_x64_128_test(123, "",                             "4cd9597081679d1abd92f8784bace33d");

    printf("Total tests: %i\nPassed: %i\nFailed: %i\n", passed + failed, passed, failed);
    if (failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

# Murmur3

Murmur3 is a (mostly) C89 compliant port of Austin Appleby's [MurmurHash3](https://github.com/aappleby/smhasher/wiki/MurmurHash3).

Murmur3 is a non-cryptographic hash designed to be fast and collision-resistant for most "typical" data sets.

The x86 and x64 versions do _not_ produce the same results, as the
algorithms are optimized for their respective platforms. You can still
compile and run any of them on any platform, but your performance with
the non-native version will be less than optimal.

This implementation is C89 compliant with a few notable exceptions. It
requires the exact width integer types uint8_t, uint32_t, and uint64_t
as described in the ISO/IEC 9899:1999 standard, Section 7.18.1.1. It
also requires UINT64_C as described in Section 7.18.4.1. The macro
must not use LL to be C89 compliant. The developer must supply their own
stdint.h header if none is provided by their compiler vendor.

The hash should work on big-endian machines, although I haven't verified this myself.
As the code is fine-tuned for x86 processors, it will likely be much slower
on big-endian machines.

## License

All of this code is in the public domain.

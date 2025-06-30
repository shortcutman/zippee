
//------------------------------------------------------------------------------
// crc32.cpp
//------------------------------------------------------------------------------

#include "crc32.hpp"

#include <array>

/*

I read a lot on how CRC works. I didn't think I could implement it, but I think
it gave me better understanding on its mathematical basis, and what the
optimised version below is in fact doing compared to simpler examples. Some of
the resources I used which were very helpful:
https://github.com/google/wuffs/tree/main/std/crc32
https://create.stephan-brumme.com/crc32/
https://en.wikipedia.org/wiki/Cyclic_redundancy_check

The below implementation is copied and adapted (for C++23) from
https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#CRC-32_example

*/

namespace {
    constexpr std::array<uint32_t, 256> generate_crc_table() {
        std::array<uint32_t, 256> table;

        uint32_t crc32 = 1;
        for (unsigned int i = 128; i; i >>= 1) {
            crc32 = (crc32 >> 1) ^ (crc32 & 1 ? 0xEDB88320 : 0);
            for (unsigned int j = 0; j < 256; j += 2*i) {
                table[i + j] = crc32 ^ table[j];
            }
        }

        return table;
    }

    std::array<uint32_t, 256> crc_table = generate_crc_table();
}

uint32_t zip::crc32(const std::vector<std::byte>& data) {
    uint32_t crc32 = 0xffffffff;

    for (auto b : data) {
        crc32 ^= std::to_integer<uint32_t>(b);
        crc32 = (crc32 >> 8) ^ crc_table[crc32 & 0xff];
    }

    crc32 ^= 0xffffffff;
    return crc32;
}

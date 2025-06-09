
//------------------------------------------------------------------------------
// deflate.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace deflate {

enum class BType : std::underlying_type_t<std::byte> {
    NoCompression = 0b00,
    FixedHuffmanCodes = 0b01,
    DynamicHuffmanCodes = 0b10,
    ReservedError = 0b11
};

std::vector<std::byte> decompress(std::span<std::byte> data);

bool is_bfinal(std::span<std::byte> data, uint8_t bit_offset);
BType get_btype(std::span<std::byte> data, uint8_t bit_offset);

}

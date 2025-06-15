
//------------------------------------------------------------------------------
// deflate.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <span>
#include <vector>
#include <map>

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

struct DynamicHeader {
    size_t hlit = 0;
    size_t hdist = 0;
    size_t hclen = 0;
    std::array<size_t, 19> code_length_codes{};
    std::map<size_t, size_t> coodes;
};

std::vector<size_t> dynamic_header_code_lengths(std::span<std::byte> data, size_t bit_offset);
std::map<size_t, size_t> bitlengths_to_huffman(const std::vector<size_t>& bitlengths);

}

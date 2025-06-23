
//------------------------------------------------------------------------------
// deflate.hpp
//------------------------------------------------------------------------------

#pragma once

#include "bitspan.hpp"

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

bool is_bfinal(zippee::bitspan& data);
BType get_btype(zippee::bitspan& data);

void dynamic_block(zippee::bitspan& data);
std::vector<size_t> dynamic_header_code_lengths(size_t count, zippee::bitspan& data);

struct HuffmanCode {
    size_t code;
    size_t code_length;
    size_t symbol;

    bool operator==(const HuffmanCode& a) const = default;
    bool operator<(const HuffmanCode& b) const {
        return this->code_length < b.code_length;
    }
};
std::vector<HuffmanCode> bitlengths_to_huffman(const std::vector<size_t>& bitlengths);
std::vector<HuffmanCode> reverse_codes(const std::vector<HuffmanCode>& codes);
size_t get_symbol_for_code(const std::vector<HuffmanCode>& codes, zippee::bitspan& data);

std::vector<size_t> read_code_length_seq(size_t count, const std::vector<HuffmanCode>& codes, zippee::bitspan& data);
void duplicate_string(std::vector<std::byte>& data, size_t length, size_t distance);

}

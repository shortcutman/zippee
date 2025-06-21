
//------------------------------------------------------------------------------
// deflate.cpp
//------------------------------------------------------------------------------

#include "deflate.hpp"

#include <utility>
#include <print>

namespace {
    std::byte get_offset_byte(std::span<std::byte> data, uint8_t bit_offset) {
        size_t byte_offset = 0;
        if (bit_offset > 7) {
            byte_offset = bit_offset / 8;
            bit_offset %= 8;
        }

        std::byte b = data[byte_offset] >> bit_offset;

        if (data.size() > (byte_offset + 1)) {
            b |= data[byte_offset + 1] << (8 - bit_offset);
        }

        return b;
    }

    std::vector<std::byte> get_offset_bytes(std::span<std::byte> data, uint8_t bit_offset, size_t bytes) {
        std::vector<std::byte> read_bytes;

        while (read_bytes.size() < bytes) {
            read_bytes.push_back(get_offset_byte(data, bit_offset + read_bytes.size() * 8));
        }

        return read_bytes;
    }

    
    size_t bits_to_qty_bytes(size_t bits) {
        return (bits + (bits % 8)) / 8;
    }
}

std::vector<std::byte> deflate::decompress(std::span<std::byte> data) {
    std::vector<std::byte> decompressed;
    zippee::bitspan bits(data);
    
    bool isLast = is_bfinal(bits);
    switch (get_btype(bits)) {
        case BType::NoCompression:
        {
            throw std::runtime_error("No compression unimplemented.");
        }
        break;

        case BType::FixedHuffmanCodes:
        {
            throw std::runtime_error("Fixed Huffman codes unimplemented.");
        }
        break;

        case BType::DynamicHuffmanCodes:
        {
            auto code_length_bytes = dynamic_header_code_lengths(data, 3);
        }
        break;

        case BType::ReservedError:
        {
            throw std::runtime_error("Reserved block type unhandled.");
        }
        break;
    }

    return decompressed;
}

bool deflate::is_bfinal(zippee::bitspan data) {
    auto flag_bit = data.read_bits(1);
    return flag_bit;
}

deflate::BType deflate::get_btype(zippee::bitspan data) {
    auto type_bits = data.read_bits(2);
    return BType(std::to_underlying<std::byte>(std::byte{static_cast<uint8_t>(type_bits)}));
}

std::vector<size_t> deflate::dynamic_header_code_lengths(zippee::bitspan data) {
    const std::array<size_t, 19> code_length_idx_to_alphabet = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    uint8_t hlit = data.read_bits(5) + 257;
    uint8_t hdist = data.read_bits(5) + 1;
    uint8_t hclen = data.read_bits(4);

    size_t hclen_qty = hclen + 4;
    std::vector<size_t> code_lengths(19, 0);

    for (size_t i = 0; i < hclen_qty; i++) {
        size_t val = data.read_bits(3);
        code_lengths[code_length_idx_to_alphabet[i]] = val;
    }

    return code_lengths;
}

std::vector<deflate::HuffmanCode>
deflate::bitlengths_to_huffman(const std::vector<size_t>& bitlengths) {
    std::vector<size_t> bl_count(bitlengths.size(), 0);
    size_t max_bits = 0;
    size_t max_codes = 0;
    //count the number of codes for each code length
    for (size_t i = 0; i < bitlengths.size(); i++) {
        if (bitlengths[i] == 0) continue;

        bl_count[bitlengths[i]]++;
        max_bits = std::max(max_bits, bitlengths[i]);
        max_codes++;
    }

    //find the numerical value of the smallest code for each code length
    size_t code = 0;
    std::vector<size_t> next_code(bitlengths.size(), 0);
    for (size_t bits = 1; bits <= max_bits; bits++) {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    //assign numerical values to all codes
    std::vector<HuffmanCode> huffman_to_symbol;
    for (size_t n = 0; n < bitlengths.size(); n++) {
        auto bit_length = bitlengths[n];
        if (bit_length != 0) {
            huffman_to_symbol.push_back(HuffmanCode{
                .code = next_code[bit_length],
                .code_length = bit_length,
                .symbol = n
            });
            next_code[bit_length]++;
        }
    }

    return huffman_to_symbol;
}
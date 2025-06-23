
//------------------------------------------------------------------------------
// deflate.cpp
//------------------------------------------------------------------------------

#include "deflate.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

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
            dynamic_block(bits);
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

bool deflate::is_bfinal(zippee::bitspan& data) {
    auto flag_bit = data.read_bits(1);
    return flag_bit;
}

deflate::BType deflate::get_btype(zippee::bitspan& data) {
    auto type_bits = data.read_bits(2);
    return BType(std::to_underlying<std::byte>(std::byte{static_cast<uint8_t>(type_bits)}));
}

void deflate::dynamic_block(zippee::bitspan& data) {
    auto literal_count = data.read_bits(5) + 257;
    auto distance_count = data.read_bits(5) + 1;
    auto code_length_count = data.read_bits(4) + 4;

    auto code_length_bytes = dynamic_header_code_lengths(code_length_count, data);
    auto huffman_table = bitlengths_to_huffman(code_length_bytes);

    std::vector<size_t> lit_code_lengths = read_code_length_seq(literal_count, huffman_table, data);
    std::vector<size_t> dist_values = read_code_length_seq(distance_count, huffman_table, data);
}

std::vector<size_t> deflate::dynamic_header_code_lengths(size_t count, zippee::bitspan& data) {
    const std::array<size_t, 19> code_length_idx_to_alphabet = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };
    std::vector<size_t> code_lengths(code_length_idx_to_alphabet.size(), 0);

    for (size_t i = 0; i < count; i++) {
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

    std::sort(huffman_to_symbol.begin(), huffman_to_symbol.end());
    huffman_to_symbol = reverse_codes(huffman_to_symbol);

    return huffman_to_symbol;
}

std::vector<deflate::HuffmanCode> deflate::reverse_codes(const std::vector<HuffmanCode>& codes) {
    std::vector<HuffmanCode> reversed_codes;

    for (auto& item : codes) {
        auto reversed = item;
        auto original = item;
        reversed.code = 0;

        while (original.code_length > 0) {
            reversed.code <<= 1;
            reversed.code |= original.code & 0x1;
            original.code >>= 1;
            original.code_length--;
        }

        reversed_codes.push_back(reversed);
    }

    return reversed_codes;
}

size_t deflate::get_symbol_for_code(const std::vector<HuffmanCode>& codes, zippee::bitspan& data) {
    assert(std::is_sorted(codes.begin(), codes.end()));

    for (auto& code : codes) {
        auto bits = data.peek_bits(code.code_length);
        if (bits == code.code) {
            data.read_bits(code.code_length);
            return code.symbol;
        }
    }

    throw std::runtime_error("Couldn't find a matching code.");

    return 0;
}

std::vector<size_t> deflate::read_code_length_seq(size_t count, const std::vector<HuffmanCode>& codes, zippee::bitspan& data) {
    std::vector<size_t> code_length_seq;
    code_length_seq.reserve(count);

    for (size_t i = 0; i < count;) {
        auto val = get_symbol_for_code(codes, data);
        size_t repeat_count = 0;

        if (val < 16) {
            repeat_count = 1;
        } else if (val == 16) {
            repeat_count = data.read_bits(2) + 3;
            val = code_length_seq.back();
        } else if (val == 17) {
            repeat_count = data.read_bits(3) + 3;
            val = 0;
        } else if (val == 18) {
            repeat_count = data.read_bits(7) + 11;
            val = 0;
        } else {
            throw std::runtime_error("Unexpected symbol.");
        }

        code_length_seq.insert(code_length_seq.end(), repeat_count, val);
        i += repeat_count;

        assert(i == code_length_seq.size()); //this should always line up
    }

    return code_length_seq;
}

void deflate::duplicate_string(std::vector<std::byte>& data, size_t length, size_t distance) {
    assert(distance > 0 && distance <= 32768);

    const size_t startIdx = data.size() - distance;
    const size_t endIdx = data.size() - 1;
    for (size_t i = 0; i < length; i++) {
        auto copyIdx = startIdx + (i % (endIdx - startIdx + 1));
        data.push_back(data[copyIdx]);
    }
}
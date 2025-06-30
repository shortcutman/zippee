
//------------------------------------------------------------------------------
// deflate.cpp
//------------------------------------------------------------------------------

#include "deflate.hpp"

#include <algorithm>
#include <cassert>
#include <print>
#include <utility>

namespace {
    std::vector<deflate::HuffmanCode> fixed_huffman_lit_codelengths()  {
        std::vector<size_t> code_lengths;

        for (size_t i = 0; i < 288; i++) {
            if (i < 144) {
                code_lengths.push_back(8);
            } else if (i < 256) {
                code_lengths.push_back(9);
            } else if (i < 280) {
                code_lengths.push_back(7);
            } else if (i < 288) {
                code_lengths.push_back(8);
            }
        }

        assert(code_lengths.size() == 288);

        return deflate::bitlengths_to_huffman(code_lengths);
    }

    std::vector<deflate::HuffmanCode> fixed_huffman_dist_codelengths() {
        return deflate::bitlengths_to_huffman(std::vector<size_t>(32, 5));
    }
}

std::vector<std::byte> deflate::decompress(std::span<std::byte> data) {
    std::vector<std::byte> decompressed;
    zippee::bitspan bits(data);

    bool isLast = true;

    do {
        isLast = is_bfinal(bits);
        switch (get_btype(bits)) {
            case BType::NoCompression:
            {
                std::println("Reading uncompressed block; note, untested!");
                uncompressed_block(bits, decompressed);
            }
            break;

            case BType::FixedHuffmanCodes:
            {
                fixed_block(bits, decompressed);
            }
            break;

            case BType::DynamicHuffmanCodes:
            {
                dynamic_block(bits, decompressed);
            }
            break;

            case BType::ReservedError:
            {
                std::println("Reading reserved block; likely corruption!");
                throw std::runtime_error("Reserved block type unhandled.");
            }
            break;
        }
    } while (!isLast);

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

void deflate::uncompressed_block(zippee::bitspan& data, std::vector<std::byte>& output) {
    data.round_to_next_byte();
    auto len = data.read_bits(16);
    auto nlen = data.read_bits(16);

    if (len != ~nlen) {
        throw std::runtime_error("LEN and NLEN do not match.");
    }

    auto uncompressed_data_span = data.to_span().subspan(0, len);
    output.insert(output.end(), uncompressed_data_span.begin(), uncompressed_data_span.end());
}

void deflate::fixed_block(zippee::bitspan& data, std::vector<std::byte>& output) {
    auto lit_huffman_table = fixed_huffman_lit_codelengths();
    auto dist_huffman_table = fixed_huffman_dist_codelengths();

    decompress_huffman(data, output, lit_huffman_table, dist_huffman_table);
}

void deflate::dynamic_block(zippee::bitspan& data, std::vector<std::byte>& output) {
    auto literal_count = data.read_bits(5) + 257;
    auto distance_count = data.read_bits(5) + 1;
    auto code_length_count = data.read_bits(4) + 4;

    auto code_length_bytes = dynamic_header_code_lengths(code_length_count, data);
    auto huffman_table = bitlengths_to_huffman(code_length_bytes);

    std::vector<size_t> lit_code_lengths = read_code_length_seq(literal_count, huffman_table, data);
    std::vector<size_t> dist_code_lengths = read_code_length_seq(distance_count, huffman_table, data);

    auto lit_huffman_table = bitlengths_to_huffman(lit_code_lengths);
    auto dist_huffman_table = bitlengths_to_huffman(dist_code_lengths);

    decompress_huffman(data, output, lit_huffman_table, dist_huffman_table);
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

void deflate::decompress_huffman(
    zippee::bitspan& data,
    std::vector<std::byte>& output,
    std::vector<deflate::HuffmanCode>& lit_table,
    std::vector<deflate::HuffmanCode>& dist_table) {
    while (true) {
        auto symbol = get_symbol_for_code(lit_table, data);
        if (symbol < 256) {
            output.push_back(std::byte{static_cast<uint8_t>(symbol)});
        }
        else if (symbol == 256) {
            break; //end of block
        } else if (symbol > 256 && symbol < 286) {
            //distance etc
            auto lgth_and_dist = read_length_and_distance(symbol, dist_table, data);
            duplicate_string(output, std::get<0>(lgth_and_dist), std::get<1>(lgth_and_dist));
        } else {
            throw std::runtime_error("Non compliant symbol.");
        }
    }
}

std::vector<deflate::HuffmanCode>
deflate::bitlengths_to_huffman(const std::vector<size_t>& bitlengths) {
    std::vector<size_t> bl_count(bitlengths.size(), 0);
    size_t max_bits = 0;
    //count the number of codes for each code length
    for (size_t i = 0; i < bitlengths.size(); i++) {
        if (bitlengths[i] == 0) continue;

        bl_count[bitlengths[i]]++;
        max_bits = std::max(max_bits, bitlengths[i]);
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

std::tuple<size_t, size_t> deflate::read_length_and_distance(
    size_t symbol,
    const std::vector<HuffmanCode>& distance_codes,
    zippee::bitspan& data) {
    assert(symbol > 256 && symbol < 286);

    const std::array<uint16_t, 30> length_extras = {
        0, //padding
        0, 0, 0, 0, 0, 0, 0, 0, // no extras
        1, 1, 1, 1, // 1-bit extras
        2, 2, 2, 2, // 2-bit extras
        3, 3, 3, 3, // 3-bit extras
        4, 4, 4, 4, // 4-bit extras
        5, 5, 5, 5, // 5-bit extras
        0 // no extras
    };
    const std::array<uint16_t, 30> length_starts = {
        0, //padding
        3, 4, 5, 6, 7, 8, 9, 10, // no extras
        11, 13, 15, 17, // 1-bit extras
        19, 23, 27, 31, // 2-bit extras
        35, 43, 51, 59, // 3-bit extras
        67, 83, 99, 115, // 4-bit extras
        131, 163, 195, 227, // 5-bit extras
        258 // no extras
    };

    size_t length = length_starts[symbol & 0xff];
    auto length_extra = data.read_bits(length_extras[symbol & 0xff]);
    length += length_extra;

    const std::array<std::tuple<uint16_t, uint16_t>, 30> distance_table = {{
        {0, 1}, {0, 2}, {0, 3}, {0, 4}, // no extras
        {1, 5}, {1, 7}, // 1-bit extras
        {2, 9}, {2, 13}, // 2-bit extras
        {3, 17}, {3, 25}, // 3-bit extras
        {4, 33}, {4, 49}, // 4-bit extras
        {5, 65}, {5, 97}, // 5-bit extras
        {6, 129}, {6, 193}, // 6-bit extras
        {7, 257}, {7, 385}, // 7-bit extras
        {8, 513}, {8, 769}, // 8-bit extras
        {9, 1025}, {9, 1537}, // 9-bit extras
        {10, 2049}, {10, 3073}, // 10-bit extras
        {11, 4097}, {11, 6145}, // 11-bit extras
        {12, 8193}, {12, 12289}, // 12-bit extras
        {13, 16385}, {13, 24577}, // 13-bit extras
    }};

    auto distance_symbol = get_symbol_for_code(distance_codes, data);
    size_t distance = std::get<1>(distance_table[distance_symbol]);
    auto distance_extra = data.read_bits(std::get<0>(distance_table[distance_symbol]));
    distance += distance_extra;

    return {length, distance};
}

void deflate::duplicate_string(std::vector<std::byte>& data, size_t length, size_t distance) {
    assert(distance > 0 && distance <= 32768);
    assert(distance < data.size());

    const size_t startIdx = data.size() - distance;
    const size_t endIdx = data.size() - 1;
    for (size_t i = 0; i < length; i++) {
        auto copyIdx = startIdx + (i % (endIdx - startIdx + 1));
        data.push_back(data[copyIdx]);
    }
}
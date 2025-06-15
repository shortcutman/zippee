
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
    
    bool isLast = is_bfinal(data, 0);
    switch (get_btype(data, 0)) {
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
            auto header = parse_dynamic_header(data, 3);
            // for (size_t n = 0; n <= 19; n++) {
            //     size_t len = 
            // }
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

bool deflate::is_bfinal(std::span<std::byte> data, uint8_t bit_offset) {
    auto flag_byte = get_offset_byte(data, bit_offset);

    if ((flag_byte & std::byte{0x01}) == std::byte{0x01}) {
        return true;
    }

    return false;
}

deflate::BType deflate::get_btype(std::span<std::byte> data, uint8_t bit_offset) {
    auto flag_byte = get_offset_byte(data, bit_offset);
    flag_byte &= std::byte{0x06};
    
    flag_byte >>= 1;
    return BType(std::to_underlying<std::byte>(flag_byte));
}

deflate::DynamicHeader deflate::parse_dynamic_header(std::span<std::byte> data, size_t bit_offset) {
    DynamicHeader header;

    std::array<size_t, 19> code_length_idx_to_alphabet = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    header.hlit = std::to_integer<size_t>(get_offset_byte(data, bit_offset) & std::byte{0x1f}) + 257;
    header.hdist = std::to_integer<size_t>(get_offset_byte(data, bit_offset + 5) & std::byte{0x1f}) + 1;
    header.hclen = std::to_integer<size_t>(get_offset_byte(data, bit_offset + 10) & std::byte{0x0f});

    size_t hclen_qty = header.hclen + 4;
    auto code_length_codes = get_offset_bytes(data, bit_offset + 14, bits_to_qty_bytes(hclen_qty * 3));
    // std::println("Code length codes bytes: {}", code_length_codes.size());

    for (size_t i = 0; i < hclen_qty; i++) {
        size_t val = std::to_integer<size_t>(get_offset_byte(code_length_codes, i * 3) & std::byte{0x07});
        header.code_length_codes[code_length_idx_to_alphabet[i]] = val;
    }

    // std::array<size_t, 19> bl_count{};

    // for (size_t i = 0; i < header.code_length_codes.size(); i++) {
    //     std::println("Code length: {}, length: {}", i, header.code_length_codes[i]);
    //     bl_count[i]++;
    // }

    // size_t code = 0;
    // std::array<size_t, 19> next_code{};

    // for (std::size_t val : bl_count) {
    //     std::cout << val << " ";  // Should print: 0 0 0 ... (19 times)
    // }
    // std::cout << std::endl;

    // for (size_t bits = 1; bits <= 19; bits++) {
    //     code = (code + bl_count[bits - 1]) << 1;
    //     next_code[bits] = code;
    // }

    return header;
}

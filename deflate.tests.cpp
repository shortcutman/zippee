//------------------------------------------------------------------------------
// deflate.tests.cpp
//------------------------------------------------------------------------------

#include "deflate.hpp"

#include <gtest/gtest.h>

namespace {

// https://stackoverflow.com/a/45172360
template<typename... Ts>
std::array<std::byte, sizeof...(Ts)> make_bytes(Ts&&... args) noexcept {
    return{std::byte(std::forward<Ts>(args))...};
}

}

TEST(Deflate, is_bfinal_true_zero_offset) {
    auto data = make_bytes(0x01);
    zippee::bitspan bits(data);
    EXPECT_TRUE(deflate::is_bfinal(bits));
    EXPECT_EQ(bits.bits_read(), 1);
}

TEST(Deflate, is_bfinal_false_zero_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    EXPECT_FALSE(deflate::is_bfinal(bits));
    EXPECT_EQ(bits.bits_read(), 1);
}

TEST(Deflate, is_bfinal_true_3bit_offset) {
    auto data = make_bytes(0x08);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_TRUE(deflate::is_bfinal(bits));
    EXPECT_EQ(bits.bits_read(), 4);
}

TEST(Deflate, is_bfinal_false_3bit_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_FALSE(deflate::is_bfinal(bits));
    EXPECT_EQ(bits.bits_read(), 4);
}

TEST(Deflate, is_bfinal_true_7bit_offset) {
    auto data = make_bytes(0x80);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_TRUE(deflate::is_bfinal(bits));
    EXPECT_EQ(bits.bits_read(), 8);
}

TEST(Deflate, is_bfinal_false_7bit_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_FALSE(deflate::is_bfinal(bits));
    EXPECT_EQ(bits.bits_read(), 8);
}

TEST(Deflate, get_btype_nocompression_zero_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::NoCompression);
    EXPECT_EQ(bits.bits_read(), 2);
}

TEST(Deflate, get_btype_fixed_huffman_zero_offset) {
    auto data = make_bytes(0x01);
    zippee::bitspan bits(data);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::FixedHuffmanCodes);
    EXPECT_EQ(bits.bits_read(), 2);
}

TEST(Deflate, get_btype_dynamic_huffman_zero_offset) {
    auto data = make_bytes(0x02);
    zippee::bitspan bits(data);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::DynamicHuffmanCodes);
    EXPECT_EQ(bits.bits_read(), 2);
}

TEST(Deflate, get_btype_reserved_error_zero_offset) {
    auto data = make_bytes(0x03);
    zippee::bitspan bits(data);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::ReservedError);
    EXPECT_EQ(bits.bits_read(), 2);
}

TEST(Deflate, get_btype_nocompression_3bit_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::NoCompression);
    EXPECT_EQ(bits.bits_read(), 5);
}

TEST(Deflate, get_btype_fixed_huffman_3bit_offset) {
    auto data = make_bytes(0x08);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::FixedHuffmanCodes);
    EXPECT_EQ(bits.bits_read(), 5);
}

TEST(Deflate, get_btype_dynamic_huffman_3bit_offset) {
    auto data = make_bytes(0x10);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::DynamicHuffmanCodes);
    EXPECT_EQ(bits.bits_read(), 5);
}

TEST(Deflate, get_btype_reserved_error_3bit_offset) {
    auto data = make_bytes(0x18);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::ReservedError);
    EXPECT_EQ(bits.bits_read(), 5);
}

TEST(Deflate, get_btype_nocompression_7bit_offset) {
    auto data = make_bytes(0x00, 0x00);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::NoCompression);
    EXPECT_EQ(bits.bits_read(), 9);
}

TEST(Deflate, get_btype_fixed_huffman_7bit_offset) {
    auto data = make_bytes(0x80, 0x00);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::FixedHuffmanCodes);
    EXPECT_EQ(bits.bits_read(), 9);
}

TEST(Deflate, get_btype_dynamic_huffman_7bit_offset) {
    auto data = make_bytes(0x00, 0x01);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::DynamicHuffmanCodes);
    EXPECT_EQ(bits.bits_read(), 9);
}

TEST(Deflate, get_btype_reserved_error_7bit_offset) {
    auto data = make_bytes(0x80, 0x01);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::ReservedError);
    EXPECT_EQ(bits.bits_read(), 9);
}

TEST(Deflate, dynamic_header_code_lengths) {
    auto data = make_bytes(0x6d, 0x8e, 0xb9, 0x72, 0x83, 0x30, 0x10, 0x40, 0xfb);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    auto code_lengths = deflate::dynamic_header_code_lengths(bits);

    EXPECT_EQ(code_lengths, std::vector<size_t>({
        4, 0, 5, 0, 4, 3, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 4, 3, 5
    }));
}

TEST(Deflate, bitlengths_to_huffman_3_2_2) {
    using namespace deflate;

    std::vector<size_t> bitlengths = {3, 3, 3, 3, 3, 2, 4, 4};
    auto huffman_table = bitlengths_to_huffman(bitlengths);

    auto result = std::vector<HuffmanCode>({
        {0b00, 2, 5},
        {0b010, 3, 0},
        {0b011, 3, 1},
        {0b100, 3, 2},
        {0b101, 3, 3},
        {0b110, 3, 4},
        {0b1110, 4, 6},
        {0b1111, 4, 7}
    });

    EXPECT_EQ(huffman_table, result);
}

TEST(Deflate, reverse_codes) {
    auto codes = std::vector<deflate::HuffmanCode>({
        {0b00, 2, 5},
        {0b010, 3, 0},
        {0b011, 3, 1},
        {0b100, 3, 2},
        {0b101, 3, 3},
        {0b110, 3, 4},
        {0b1110, 4, 6},
        {0b1111, 4, 7}
    });

    codes = deflate::reverse_codes(codes);

    auto reversed = std::vector<deflate::HuffmanCode>({
        {0b00, 2, 5},
        {0b010, 3, 0},
        {0b110, 3, 1},
        {0b001, 3, 2},
        {0b101, 3, 3},
        {0b011, 3, 4},
        {0b0111, 4, 6},
        {0b1111, 4, 7}
    });

    EXPECT_EQ(codes, reversed);
}

TEST(Deflate, get_code_symmetrical) {
    auto codes = std::vector<deflate::HuffmanCode>({
        {0b00, 2, 5},
        {0b010, 3, 0},
        {0b011, 3, 1},
        {0b100, 3, 2},
        {0b101, 3, 3},
        {0b110, 3, 4},
        {0b1110, 4, 6},
        {0b1111, 4, 7}
    });
    codes = deflate::reverse_codes(codes);

    auto data = make_bytes(0xa8, 0x0f);
    zippee::bitspan bits(data);
    auto symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 5);
    symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 0);
    symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 3);
    symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 7);
}

TEST(Deflate, get_code_asymmetrical) {
    auto codes = std::vector<deflate::HuffmanCode>({
        {0b00, 2, 5},
        {0b010, 3, 0},
        {0b011, 3, 1},
        {0b100, 3, 2},
        {0b101, 3, 3},
        {0b110, 3, 4},
        {0b1110, 4, 6},
        {0b1111, 4, 7}
    });
    codes = deflate::reverse_codes(codes);

    auto data = make_bytes(0xce, 0x0e);
    zippee::bitspan bits(data);
    auto symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 1);
    symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 2);
    symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 4);
    symbol = deflate::get_code(codes, bits);
    EXPECT_EQ(symbol, 6);
}

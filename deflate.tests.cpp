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
    EXPECT_TRUE(deflate::is_bfinal(data));
}

TEST(Deflate, is_bfinal_false_zero_offset) {
    auto data = make_bytes(0x00);
    EXPECT_FALSE(deflate::is_bfinal(data));
}

TEST(Deflate, is_bfinal_true_3bit_offset) {
    auto data = make_bytes(0x08);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_TRUE(deflate::is_bfinal(bits));
}

TEST(Deflate, is_bfinal_false_3bit_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_FALSE(deflate::is_bfinal(bits));
}

TEST(Deflate, is_bfinal_true_7bit_offset) {
    auto data = make_bytes(0x80);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_TRUE(deflate::is_bfinal(bits));
}

TEST(Deflate, is_bfinal_false_7bit_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_FALSE(deflate::is_bfinal(bits));
}

TEST(Deflate, get_btype_nocompression_zero_offset) {
    auto data = make_bytes(0x00);
    EXPECT_EQ(deflate::get_btype(data), deflate::BType::NoCompression);
}

TEST(Deflate, get_btype_fixed_huffman_zero_offset) {
    auto data = make_bytes(0x01);
    EXPECT_EQ(deflate::get_btype(data), deflate::BType::FixedHuffmanCodes);
}

TEST(Deflate, get_btype_dynamic_huffman_zero_offset) {
    auto data = make_bytes(0x02);
    EXPECT_EQ(deflate::get_btype(data), deflate::BType::DynamicHuffmanCodes);
}

TEST(Deflate, get_btype_reserved_error_zero_offset) {
    auto data = make_bytes(0x03);
    EXPECT_EQ(deflate::get_btype(data), deflate::BType::ReservedError);
}

TEST(Deflate, get_btype_nocompression_3bit_offset) {
    auto data = make_bytes(0x00);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::NoCompression);
}

TEST(Deflate, get_btype_fixed_huffman_3bit_offset) {
    auto data = make_bytes(0x08);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::FixedHuffmanCodes);
}

TEST(Deflate, get_btype_dynamic_huffman_3bit_offset) {
    auto data = make_bytes(0x10);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::DynamicHuffmanCodes);
}

TEST(Deflate, get_btype_reserved_error_3bit_offset) {
    auto data = make_bytes(0x18);
    zippee::bitspan bits(data);
    bits.read_bits(3);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::ReservedError);
}

TEST(Deflate, get_btype_nocompression_7bit_offset) {
    auto data = make_bytes(0x00, 0x00);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::NoCompression);
}

TEST(Deflate, get_btype_fixed_huffman_7bit_offset) {
    auto data = make_bytes(0x80, 0x00);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::FixedHuffmanCodes);
}

TEST(Deflate, get_btype_dynamic_huffman_7bit_offset) {
    auto data = make_bytes(0x00, 0x01);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::DynamicHuffmanCodes);
}

TEST(Deflate, get_btype_reserved_error_7bit_offset) {
    auto data = make_bytes(0x80, 0x01);
    zippee::bitspan bits(data);
    bits.read_bits(7);
    EXPECT_EQ(deflate::get_btype(bits), deflate::BType::ReservedError);
}

TEST(Deflate, dynamic_header_code_lengths1) {
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

    auto result = std::map<size_t, size_t>({
        {0b010, 0},
        {0b011, 1},
        {0b100, 2},
        {0b101, 3},
        {0b110, 4},
        {0b00, 5},
        {0b1110, 6},
        {0b1111, 7}
    });

    EXPECT_EQ(huffman_table, result);
}
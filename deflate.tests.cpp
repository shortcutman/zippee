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
    EXPECT_TRUE(deflate::is_bfinal(data, 0));
}

TEST(Deflate, is_bfinal_false_zero_offset) {
    auto data = make_bytes(0x00);
    EXPECT_FALSE(deflate::is_bfinal(data, 0));
}

TEST(Deflate, is_bfinal_true_3bit_offset) {
    auto data = make_bytes(0x08);
    EXPECT_TRUE(deflate::is_bfinal(data, 3));
}

TEST(Deflate, is_bfinal_false_3bit_offset) {
    auto data = make_bytes(0x00);
    EXPECT_FALSE(deflate::is_bfinal(data, 3));
}

TEST(Deflate, is_bfinal_true_7bit_offset) {
    auto data = make_bytes(0x80);
    EXPECT_TRUE(deflate::is_bfinal(data, 7));
}

TEST(Deflate, is_bfinal_false_7bit_offset) {
    auto data = make_bytes(0x00);
    EXPECT_FALSE(deflate::is_bfinal(data, 7));
}

TEST(Deflate, get_btype_nocompression_zero_offset) {
    auto data = make_bytes(0x00);
    EXPECT_EQ(deflate::get_btype(data, 0), deflate::BType::NoCompression);
}

TEST(Deflate, get_btype_fixed_huffman_zero_offset) {
    auto data = make_bytes(0x02);
    EXPECT_EQ(deflate::get_btype(data, 0), deflate::BType::FixedHuffmanCodes);
}

TEST(Deflate, get_btype_dynamic_huffman_zero_offset) {
    auto data = make_bytes(0x04);
    EXPECT_EQ(deflate::get_btype(data, 0), deflate::BType::DynamicHuffmanCodes);
}

TEST(Deflate, get_btype_reserved_error_zero_offset) {
    auto data = make_bytes(0x06);
    EXPECT_EQ(deflate::get_btype(data, 0), deflate::BType::ReservedError);
}

TEST(Deflate, get_btype_nocompression_3bit_offset) {
    auto data = make_bytes(0x00);
    EXPECT_EQ(deflate::get_btype(data, 3), deflate::BType::NoCompression);
}

TEST(Deflate, get_btype_fixed_huffman_3bit_offset) {
    auto data = make_bytes(0x10);
    EXPECT_EQ(deflate::get_btype(data, 3), deflate::BType::FixedHuffmanCodes);
}

TEST(Deflate, get_btype_dynamic_huffman_3bit_offset) {
    auto data = make_bytes(0x20);
    EXPECT_EQ(deflate::get_btype(data, 3), deflate::BType::DynamicHuffmanCodes);
}

TEST(Deflate, get_btype_reserved_error_3bit_offset) {
    auto data = make_bytes(0x30);
    EXPECT_EQ(deflate::get_btype(data, 3), deflate::BType::ReservedError);
}

TEST(Deflate, get_btype_nocompression_7bit_offset) {
    auto data = make_bytes(0x00, 0x00);
    EXPECT_EQ(deflate::get_btype(data, 7), deflate::BType::NoCompression);
}

TEST(Deflate, get_btype_fixed_huffman_7bit_offset) {
    auto data = make_bytes(0x00, 0x01);
    EXPECT_EQ(deflate::get_btype(data, 7), deflate::BType::FixedHuffmanCodes);
}

TEST(Deflate, get_btype_dynamic_huffman_7bit_offset) {
    auto data = make_bytes(0x00, 0x02);
    EXPECT_EQ(deflate::get_btype(data, 7), deflate::BType::DynamicHuffmanCodes);
}

TEST(Deflate, get_btype_reserved_error_7bit_offset) {
    auto data = make_bytes(0x00, 0x03);
    EXPECT_EQ(deflate::get_btype(data, 7), deflate::BType::ReservedError);
}

TEST(Deflate, block) {
    auto data = make_bytes(0x6d, 0x8e, 0xb9, 0x72, 0x83, 0x30, 0x10, 0x40, 0xfb);
    auto header = deflate::parse_dynamic_header(data, 3);

    EXPECT_EQ(header.hlit, 270);
    EXPECT_EQ(header.hdist, 15);
    EXPECT_EQ(header.hclen, 12);
    EXPECT_EQ(header.code_length_codes, (std::array<size_t, 19>({
        4, 0, 5, 0, 4, 3, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 4, 3, 5
    })));
}
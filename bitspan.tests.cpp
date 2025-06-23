
//------------------------------------------------------------------------------
// bitspan.tests.cpp
//------------------------------------------------------------------------------

#include "bitspan.hpp"

#include <gtest/gtest.h>

namespace {

// https://stackoverflow.com/a/45172360
template<typename... Ts>
std::array<std::byte, sizeof...(Ts)> make_bytes(Ts&&... args) noexcept {
    return{std::byte(std::forward<Ts>(args))...};
}

}

using namespace zippee;

TEST(BitSpan, peak_and_read_0bits) {
    auto data = make_bytes(0xff);
    auto span = bitspan(std::span{data});

    EXPECT_EQ(span.peek_bits(0), 0x00);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_EQ(span.read_bits(0), 0x00);
    EXPECT_EQ(span.bits_read(), 0);
}

TEST(BitSpan, peak_and_read_4bits_from_8bits) {
    auto data = make_bytes(0xff);
    auto span = bitspan(std::span{data});

    EXPECT_EQ(span.peek_bits(4), 0x0f);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_EQ(span.read_bits(4), 0x0f);
    EXPECT_EQ(span.bits_read(), 4);
}

TEST(BitSpan, peak_and_read_8bits_from_8bits) {
    auto data = make_bytes(0xff);
    auto span = bitspan(std::span{data});

    EXPECT_EQ(span.peek_bits(8), 0xff);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_EQ(span.read_bits(8), 0xff);
    EXPECT_EQ(span.bits_read(), 8);
}

TEST(BitSpan, fail_to_peek_and_read_16bits_from_8bits) {
    auto data = make_bytes(0xff);
    auto span = bitspan(std::span{data});

    EXPECT_THROW(span.peek_bits(16), std::runtime_error);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_THROW(span.read_bits(16), std::runtime_error);
    EXPECT_EQ(span.bits_read(), 0);
}

TEST(BitSpan, peak_and_read_12bits_from_16bits) {
    auto data = make_bytes(0xff, 0x88);
    auto span = bitspan(std::span{data});

    EXPECT_EQ(span.peek_bits(12), 0x8ff);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_EQ(span.read_bits(12), 0x8ff);
    EXPECT_EQ(span.bits_read(), 12);
}

TEST(BitSpan, peak_and_read_16bits_from_16bits) {
    auto data = make_bytes(0xff, 0x88);
    auto span = bitspan(std::span{data});

    EXPECT_EQ(span.peek_bits(16), 0x88ff);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_EQ(span.read_bits(16), 0x88ff);
    EXPECT_EQ(span.bits_read(), 16);
}

TEST(BitSpan, fail_to_peak_and_read_20bits_from_16bits) {
    auto data = make_bytes(0xff, 0x88);
    auto span = bitspan(std::span{data});

    EXPECT_THROW(span.peek_bits(20), std::runtime_error);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_THROW(span.read_bits(20), std::runtime_error);
    EXPECT_EQ(span.bits_read(), 0);
}

TEST(BitSpan, subsequent_peeks_and_reads) {
    auto data = make_bytes(0xf8, 0xf8);
    auto span = bitspan(std::span{data});

    EXPECT_EQ(span.peek_bits(4), 0x8);
    EXPECT_EQ(span.bits_read(), 0);
    EXPECT_EQ(span.read_bits(4), 0x8);
    EXPECT_EQ(span.bits_read(), 4);
    EXPECT_EQ(span.peek_bits(4), 0xf);
    EXPECT_EQ(span.bits_read(), 4);
    EXPECT_EQ(span.read_bits(4), 0xf);
    EXPECT_EQ(span.bits_read(), 8);

    EXPECT_EQ(span.peek_bits(4), 0x8);
    EXPECT_EQ(span.bits_read(), 8);
    EXPECT_EQ(span.read_bits(4), 0x8);
    EXPECT_EQ(span.bits_read(), 12);
    EXPECT_EQ(span.peek_bits(4), 0xf);
    EXPECT_EQ(span.bits_read(), 12);
    EXPECT_EQ(span.read_bits(4), 0xf);
    EXPECT_EQ(span.bits_read(), 16);
}
//------------------------------------------------------------------------------
// zip.tests.cpp
//------------------------------------------------------------------------------

#include "zip.hpp"

#include <span>

#include <gtest/gtest.h>

namespace {

// https://stackoverflow.com/a/45172360
template<typename... Ts>
std::array<std::byte, sizeof...(Ts)> make_bytes(Ts&&... args) noexcept {
    return{std::byte(std::forward<Ts>(args))...};
}

}

TEST(ZipTests, search_for_eocd_empty) {
    auto data = make_bytes(
        0x50, 0x4B, 0x05, 0x06,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    );

    auto result = zip::search_for_eocd(std::span<std::byte>(data));
    EXPECT_TRUE(result.has_value());
    auto eocd = result.value();

    EXPECT_EQ(eocd.disk_number, 0);
    EXPECT_EQ(eocd.num_disk_with_central_directory_start, 0);
    EXPECT_EQ(eocd.total_num_entries_central_directory_this_disk, 0);
    EXPECT_EQ(eocd.total_num_entries_central_directory, 0);
    EXPECT_EQ(eocd.size_central_directory, 0);
    EXPECT_EQ(eocd.offset_start_central_directory, 0);
    EXPECT_EQ(eocd.comment_length, 0);
    EXPECT_TRUE(eocd.comment.empty());
}

TEST(ZipTests, search_for_eocd_values_no_comment) {
    auto data = make_bytes(
        0x50, 0x4B, 0x05, 0x06,
        0x01, 0x01,
        0x01, 0x01,
        0x01, 0x01,
        0x01, 0x01,
        0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01,
        0x00, 0x00
    );

    auto result = zip::search_for_eocd(std::span<std::byte>(data));
    EXPECT_TRUE(result.has_value());
    auto eocd = result.value();

    EXPECT_EQ(eocd.disk_number, 257);
    EXPECT_EQ(eocd.num_disk_with_central_directory_start, 257);
    EXPECT_EQ(eocd.total_num_entries_central_directory_this_disk, 257);
    EXPECT_EQ(eocd.total_num_entries_central_directory, 257);
    EXPECT_EQ(eocd.size_central_directory, 16843009);
    EXPECT_EQ(eocd.offset_start_central_directory, 16843009);
    EXPECT_EQ(eocd.comment_length, 0);
    EXPECT_TRUE(eocd.comment.empty());
}

TEST(ZipTests, search_for_eocd_values_comment) {
    auto data = make_bytes(
        0x50, 0x4B, 0x05, 0x06,
        0x01, 0x01,
        0x01, 0x01,
        0x01, 0x01,
        0x01, 0x01,
        0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01,
        0x05, 0x00,
        'h', 'e', 'l', 'l', 'o'
    );

    auto result = zip::search_for_eocd(std::span<std::byte>(data));
    EXPECT_TRUE(result.has_value());
    auto eocd = result.value();

    EXPECT_EQ(eocd.disk_number, 257);
    EXPECT_EQ(eocd.num_disk_with_central_directory_start, 257);
    EXPECT_EQ(eocd.total_num_entries_central_directory_this_disk, 257);
    EXPECT_EQ(eocd.total_num_entries_central_directory, 257);
    EXPECT_EQ(eocd.size_central_directory, 16843009);
    EXPECT_EQ(eocd.offset_start_central_directory, 16843009);
    EXPECT_EQ(eocd.comment_length, 5);
    EXPECT_EQ(eocd.comment, "hello");
}

TEST(ZipTests, search_for_eocd_failure_length) {
    auto data = make_bytes(
        0x00
    );

    auto result = zip::search_for_eocd(std::span<std::byte>(data));
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Not enough bytes for an EOCD.");
}

TEST(ZipTests, search_for_eocd_failure_no_signature) {
    auto data = make_bytes(
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    );

    auto result = zip::search_for_eocd(std::span<std::byte>(data));
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Unable to find EOCD.");
}

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

TEST(ZipTests, read_no_central_directory_header) {
    auto data = make_bytes(
    );

    auto result = zip::read_central_directory_headers(std::span{data});
    EXPECT_TRUE(result.empty());
}

TEST(ZipTests, read_one_central_directory_header_dynamic_content) {
    auto data = make_bytes(
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00,
        'h', 'e', 'l', 'l', 'o',
        0x01, 0x02, 0x03, 0x04, 0x05,
        't', 'h', 'e', 'r', 'e'
    );

    auto result = zip::read_central_directory_headers(std::span{data});
    ASSERT_FALSE(result.empty());

    auto frontHeader = result.front();
    EXPECT_EQ(frontHeader.signature, 0x02014b50);
    EXPECT_EQ(frontHeader.version_made_by, 0x0314);
    EXPECT_EQ(frontHeader.version_needed, 0x14);
    EXPECT_EQ(frontHeader.general_purpose_bit_flag, 0x08);
    EXPECT_EQ(frontHeader.compression_method, 0x08);
    EXPECT_EQ(frontHeader.last_mod_file_time, 0x581a);
    EXPECT_EQ(frontHeader.last_mod_file_date, 0x5a7f);
    EXPECT_EQ(frontHeader.crc_32, 0x54809df5);
    EXPECT_EQ(frontHeader.compressed_size, 0x0164);
    EXPECT_EQ(frontHeader.uncompressed_size, 0x0202);
    EXPECT_EQ(frontHeader.file_name_length, 0x05);
    EXPECT_EQ(frontHeader.extra_field_length, 0x05);
    EXPECT_EQ(frontHeader.file_comment_length, 0x05);
    EXPECT_EQ(frontHeader.disk_number_start, 0x00);
    EXPECT_EQ(frontHeader.internal_file_attributes, 0x00);
    EXPECT_EQ(frontHeader.external_file_attributes, 0x81a40000);
    EXPECT_EQ(frontHeader.relative_offset_of_local_header, 0x00);
    EXPECT_EQ(frontHeader.file_name, "hello");
    EXPECT_TRUE(std::equal(
        frontHeader.extra_field.begin(),
        frontHeader.extra_field.end(),
        make_bytes(0x01, 0x02, 0x03, 0x04, 0x05).begin()));
    EXPECT_EQ(frontHeader.file_comment, "there");
}

TEST(ZipTests, read_one_central_directory_header_no_dynamic_content) {
    auto data = make_bytes(
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00
    );

    auto result = zip::read_central_directory_headers(std::span{data});
    ASSERT_FALSE(result.empty());

    auto frontHeader = result.front();
    EXPECT_EQ(frontHeader.signature, 0x02014b50);
    EXPECT_EQ(frontHeader.version_made_by, 0x0314);
    EXPECT_EQ(frontHeader.version_needed, 0x14);
    EXPECT_EQ(frontHeader.general_purpose_bit_flag, 0x08);
    EXPECT_EQ(frontHeader.compression_method, 0x08);
    EXPECT_EQ(frontHeader.last_mod_file_time, 0x581a);
    EXPECT_EQ(frontHeader.last_mod_file_date, 0x5a7f);
    EXPECT_EQ(frontHeader.crc_32, 0x54809df5);
    EXPECT_EQ(frontHeader.compressed_size, 0x0164);
    EXPECT_EQ(frontHeader.uncompressed_size, 0x0202);
    EXPECT_EQ(frontHeader.file_name_length, 0x00);
    EXPECT_EQ(frontHeader.extra_field_length, 0x00);
    EXPECT_EQ(frontHeader.file_comment_length, 0x00);
    EXPECT_EQ(frontHeader.disk_number_start, 0x00);
    EXPECT_EQ(frontHeader.internal_file_attributes, 0x00);
    EXPECT_EQ(frontHeader.external_file_attributes, 0x81a40000);
    EXPECT_EQ(frontHeader.relative_offset_of_local_header, 0x00);
    EXPECT_TRUE(frontHeader.file_name.empty());
    EXPECT_TRUE(frontHeader.extra_field.empty());
    EXPECT_TRUE(frontHeader.file_comment.empty());
}

TEST(ZipTests, read_two_central_directory_header_dynamic_content) {
    auto data = make_bytes(
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00,
        'h', 'e', 'l', 'l', 'o',
        0x01, 0x02, 0x03, 0x04, 0x05,
        't', 'h', 'e', 'r', 'e',
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00,
        'h', 'e', 'l', 'l', 'o',
        0x01, 0x02, 0x03, 0x04, 0x05,
        't', 'h', 'e', 'r', 'e'
    );

    auto result = zip::read_central_directory_headers(std::span{data});
    ASSERT_EQ(result.size(), 2);

    for (auto& header : result) {
        EXPECT_EQ(header.signature, 0x02014b50);
        EXPECT_EQ(header.version_made_by, 0x0314);
        EXPECT_EQ(header.version_needed, 0x14);
        EXPECT_EQ(header.general_purpose_bit_flag, 0x08);
        EXPECT_EQ(header.compression_method, 0x08);
        EXPECT_EQ(header.last_mod_file_time, 0x581a);
        EXPECT_EQ(header.last_mod_file_date, 0x5a7f);
        EXPECT_EQ(header.crc_32, 0x54809df5);
        EXPECT_EQ(header.compressed_size, 0x0164);
        EXPECT_EQ(header.uncompressed_size, 0x0202);
        EXPECT_EQ(header.file_name_length, 0x05);
        EXPECT_EQ(header.extra_field_length, 0x05);
        EXPECT_EQ(header.file_comment_length, 0x05);
        EXPECT_EQ(header.disk_number_start, 0x00);
        EXPECT_EQ(header.internal_file_attributes, 0x00);
        EXPECT_EQ(header.external_file_attributes, 0x81a40000);
        EXPECT_EQ(header.relative_offset_of_local_header, 0x00);
        EXPECT_EQ(header.file_name, "hello");
        EXPECT_TRUE(std::equal(
            header.extra_field.begin(),
            header.extra_field.end(),
            make_bytes(0x01, 0x02, 0x03, 0x04, 0x05).begin()));
        EXPECT_EQ(header.file_comment, "there");
    }
}

TEST(ZipTests, read_two_central_directory_header_no_dynamic_content) {
    auto data = make_bytes(
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00,
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00
    );

    auto result = zip::read_central_directory_headers(std::span{data});
    ASSERT_EQ(result.size(), 2);

    auto frontHeader = result.front();

    for (auto& header : result) {
        EXPECT_EQ(header.signature, 0x02014b50);
        EXPECT_EQ(header.version_made_by, 0x0314);
        EXPECT_EQ(header.version_needed, 0x14);
        EXPECT_EQ(header.general_purpose_bit_flag, 0x08);
        EXPECT_EQ(header.compression_method, 0x08);
        EXPECT_EQ(header.last_mod_file_time, 0x581a);
        EXPECT_EQ(header.last_mod_file_date, 0x5a7f);
        EXPECT_EQ(header.crc_32, 0x54809df5);
        EXPECT_EQ(header.compressed_size, 0x0164);
        EXPECT_EQ(header.uncompressed_size, 0x0202);
        EXPECT_EQ(header.file_name_length, 0x00);
        EXPECT_EQ(header.extra_field_length, 0x00);
        EXPECT_EQ(header.file_comment_length, 0x00);
        EXPECT_EQ(header.disk_number_start, 0x00);
        EXPECT_EQ(header.internal_file_attributes, 0x00);
        EXPECT_EQ(header.external_file_attributes, 0x81a40000);
        EXPECT_EQ(header.relative_offset_of_local_header, 0x00);
        EXPECT_TRUE(header.file_name.empty());
        EXPECT_TRUE(header.extra_field.empty());
        EXPECT_TRUE(header.file_comment.empty());
    }
}

TEST(ZipTests, read_two_central_directory_header_mixed_content) {
    auto data = make_bytes(
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x05, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00,
        'h', 'e', 'l', 'l', 'o',
        0x01, 0x02, 0x03, 0x04, 0x05,
        't', 'h', 'e', 'r', 'e',
        0x50, 0x4B, 0x01, 0x02,
        0x14, 0x03,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0xF5, 0x9D, 0x80, 0x54,
        0x64, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0xA4, 0x81,
        0x00, 0x00, 0x00, 0x00
    );

    auto result = zip::read_central_directory_headers(std::span{data});
    ASSERT_FALSE(result.empty());

    auto frontHeader = result.front();
    EXPECT_EQ(frontHeader.signature, 0x02014b50);
    EXPECT_EQ(frontHeader.version_made_by, 0x0314);
    EXPECT_EQ(frontHeader.version_needed, 0x14);
    EXPECT_EQ(frontHeader.general_purpose_bit_flag, 0x08);
    EXPECT_EQ(frontHeader.compression_method, 0x08);
    EXPECT_EQ(frontHeader.last_mod_file_time, 0x581a);
    EXPECT_EQ(frontHeader.last_mod_file_date, 0x5a7f);
    EXPECT_EQ(frontHeader.crc_32, 0x54809df5);
    EXPECT_EQ(frontHeader.compressed_size, 0x0164);
    EXPECT_EQ(frontHeader.uncompressed_size, 0x0202);
    EXPECT_EQ(frontHeader.file_name_length, 0x05);
    EXPECT_EQ(frontHeader.extra_field_length, 0x05);
    EXPECT_EQ(frontHeader.file_comment_length, 0x05);
    EXPECT_EQ(frontHeader.disk_number_start, 0x00);
    EXPECT_EQ(frontHeader.internal_file_attributes, 0x00);
    EXPECT_EQ(frontHeader.external_file_attributes, 0x81a40000);
    EXPECT_EQ(frontHeader.relative_offset_of_local_header, 0x00);
    EXPECT_EQ(frontHeader.file_name, "hello");
    EXPECT_TRUE(std::equal(
        frontHeader.extra_field.begin(),
        frontHeader.extra_field.end(),
        make_bytes(0x01, 0x02, 0x03, 0x04, 0x05).begin()));
    EXPECT_EQ(frontHeader.file_comment, "there");

    frontHeader = result.back();
    EXPECT_EQ(frontHeader.signature, 0x02014b50);
    EXPECT_EQ(frontHeader.version_made_by, 0x0314);
    EXPECT_EQ(frontHeader.version_needed, 0x14);
    EXPECT_EQ(frontHeader.general_purpose_bit_flag, 0x08);
    EXPECT_EQ(frontHeader.compression_method, 0x08);
    EXPECT_EQ(frontHeader.last_mod_file_time, 0x581a);
    EXPECT_EQ(frontHeader.last_mod_file_date, 0x5a7f);
    EXPECT_EQ(frontHeader.crc_32, 0x54809df5);
    EXPECT_EQ(frontHeader.compressed_size, 0x0164);
    EXPECT_EQ(frontHeader.uncompressed_size, 0x0202);
    EXPECT_EQ(frontHeader.file_name_length, 0x00);
    EXPECT_EQ(frontHeader.extra_field_length, 0x00);
    EXPECT_EQ(frontHeader.file_comment_length, 0x00);
    EXPECT_EQ(frontHeader.disk_number_start, 0x00);
    EXPECT_EQ(frontHeader.internal_file_attributes, 0x00);
    EXPECT_EQ(frontHeader.external_file_attributes, 0x81a40000);
    EXPECT_EQ(frontHeader.relative_offset_of_local_header, 0x00);
    EXPECT_TRUE(frontHeader.file_name.empty());
    EXPECT_TRUE(frontHeader.extra_field.empty());
    EXPECT_TRUE(frontHeader.file_comment.empty());
}

TEST(ZipTests, read_local_file_header_no_content) {
    auto data = make_bytes();
    auto result = zip::read_local_header(data);
    EXPECT_EQ(result.error(), "Not enough bytes for a Local File Header.");
}

TEST(ZipTests, read_local_file_header_incorrect_signature) {
    auto data = make_bytes(
        0x10, 0x20, 0x30, 0x40,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x05, 0x00,
        0x00, 0x00,
        'h', 'e', 'l', 'l', 'o'
    );

    auto result = zip::read_local_header(data);
    EXPECT_EQ(result.error(), "Local file header signature not matched.");
}

TEST(ZipTests, read_local_file_header_content) {
    auto data = make_bytes(
        0x50, 0x4B, 0x03, 0x04,
        0x14, 0x00,
        0x08, 0x00,
        0x08, 0x00,
        0x1A, 0x58,
        0x7F, 0x5A,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00,
        0x05, 0x00,
        0x00, 0x00,
        'h', 'e', 'l', 'l', 'o'
    );

    auto result = zip::read_local_header(data);
    EXPECT_TRUE(result.has_value());

    auto local_header = result.value();
    EXPECT_EQ(local_header.signature, 0x04034b50);
    EXPECT_EQ(local_header.extraction_version, 0x14);
    EXPECT_EQ(local_header.gp_bit_flag, 0x08);
    EXPECT_EQ(local_header.compression_method, 0x08);
    EXPECT_EQ(local_header.last_mod_file_time, 0x581a);
    EXPECT_EQ(local_header.last_mod_file_date, 0x5a7f);
    EXPECT_EQ(local_header.crc_32, 0x00);
    EXPECT_EQ(local_header.compressed_size, 0x00);
    EXPECT_EQ(local_header.uncompressed_size, 0x0202);
    EXPECT_EQ(local_header.file_name_length, 0x05);
    EXPECT_EQ(local_header.extra_field_length, 0x00);
    EXPECT_EQ(local_header.file_name, "hello");
    EXPECT_TRUE(local_header.extra_field.empty());
}
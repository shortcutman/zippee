
//------------------------------------------------------------------------------
// zip.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <iostream>
#include <span>
#include <vector>

namespace zip {

struct EOCD {
    uint32_t signature;
    uint16_t disk_number;
    uint16_t num_disk_with_central_directory_start;
    uint16_t total_num_entries_central_directory_this_disk;
    uint16_t total_num_entries_central_directory;
    uint32_t size_central_directory;
    uint32_t offset_start_central_directory;
    uint16_t comment_length;
    std::string comment;

    static const size_t SPEC_MIN_SIZE = 22;

    friend std::ostream& operator<<(std::ostream& os, const EOCD& s);
};

std::expected<EOCD, std::string> search_for_eocd(std::span<std::byte> data);

struct CentralDirectoryHeader {
    uint32_t signature;
    uint16_t version_made_by;
    uint16_t version_needed;
    uint16_t general_purpose_bit_flag;
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc_32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t disk_number_start;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t relative_offset_of_local_header;

    std::string file_name;
    std::string file_comment;
};

std::vector<CentralDirectoryHeader> read_central_directory_headers(std::span<std::byte> data);

}
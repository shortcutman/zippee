
//------------------------------------------------------------------------------
// zip.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <iostream>

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

std::expected<EOCD, std::string> search_for_eocd(std::istream& is);

}
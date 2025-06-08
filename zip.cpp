
//------------------------------------------------------------------------------
// zip.cpp
//------------------------------------------------------------------------------

#include "zip.hpp"

namespace {
    template<typename T, typename S>
    void read_adv(T& dest, std::span<S>& data) {
        dest = *reinterpret_cast<T*>(data.data());
        data = data.subspan(sizeof(T));
    }
}

std::ostream& zip::operator<<(std::ostream& os, const EOCD& s) {
    os << "EOCD: Signature: 0x" << std::hex << s.signature;

    os << " Size of central directory: " << std::dec << s.size_central_directory;

    if (s.comment.empty()) {
        os << " No comment";
    } else {
        os << " Comment: " << s.comment;
    }

    return os;
}

std::expected<zip::EOCD, std::string> zip::search_for_eocd(std::span<std::byte> data) {
    EOCD s;

    size_t eof = data.size();
    if (eof < EOCD::SPEC_MIN_SIZE) {
        return std::unexpected("Not enough bytes for an EOCD.");
    }

    for (ssize_t seekPos = eof - 22; seekPos >= 0; seekPos--) {
        std::span<std::byte> eocdStart(data.begin() + seekPos, data.end());

        read_adv(s.signature, eocdStart);
        if (s.signature != 0x06054b50) {
            continue;
        }

        read_adv(s.disk_number, eocdStart);
        read_adv(s.num_disk_with_central_directory_start, eocdStart);
        read_adv(s.total_num_entries_central_directory_this_disk, eocdStart);
        read_adv(s.total_num_entries_central_directory, eocdStart);
        read_adv(s.size_central_directory, eocdStart);
        read_adv(s.offset_start_central_directory, eocdStart);
        read_adv(s.comment_length, eocdStart);

        if (s.comment_length != eocdStart.size()) {
            continue;
        }

        s.comment.insert(0, reinterpret_cast<char*>(&eocdStart[0]), eocdStart.size());

        return s;
    }

    return std::unexpected("Unable to find EOCD.");
}

std::ostream& zip::operator<<(std::ostream& os, const CentralDirectoryHeader& h) {
    os << "CDH: Signature: 0x" << std::hex << h.signature;

    if (h.file_name.empty()) {
        os << " No filename";
    } else {
        os << " Filename: " << h.file_name;
    }

    return os;
}

std::vector<zip::CentralDirectoryHeader> zip::read_central_directory_headers(std::span<std::byte> data) {
    std::vector<zip::CentralDirectoryHeader> headers;

    while (!data.empty()) {
        zip::CentralDirectoryHeader h;

        read_adv(h.signature, data);
        if (h.signature != 0x02014b50) {
            return headers;
        }

        read_adv(h.version_made_by, data);
        read_adv(h.version_needed, data);
        read_adv(h.general_purpose_bit_flag, data);
        read_adv(h.compression_method, data);
        read_adv(h.last_mod_file_time, data);
        read_adv(h.last_mod_file_date, data);
        read_adv(h.crc_32, data);
        read_adv(h.compressed_size, data);
        read_adv(h.uncompressed_size, data);
        read_adv(h.file_name_length, data);
        read_adv(h.extra_field_length, data);
        read_adv(h.file_comment_length, data);
        read_adv(h.disk_number_start, data);
        read_adv(h.internal_file_attributes, data);
        read_adv(h.external_file_attributes, data);
        read_adv(h.relative_offset_of_local_header, data);

        h.file_name.insert(0, reinterpret_cast<char*>(&data[0]), h.file_name_length);
        data = data.subspan(h.file_name_length);
        h.extra_field.insert(h.extra_field.begin(), data.begin(), data.begin() + h.extra_field_length);
        data = data.subspan(h.extra_field_length);
        h.file_comment.insert(0, reinterpret_cast<char*>(&data[0]), h.file_comment_length);
        data = data.subspan(h.file_comment_length);

        headers.push_back(h);
    }

    return headers;
}

//------------------------------------------------------------------------------
// zip.cpp
//------------------------------------------------------------------------------

#include "zip.hpp"

std::ostream& zip::operator<<(std::ostream& os, const EOCD& s) {
    os << "Signature: 0x" << std::hex << s.signature;

    if (s.comment.empty()) {
        os << " No comment";
    } else {
        os << " Comment: " << s.comment;
    }

    return os;
}

std::expected<zip::EOCD, std::string> zip::search_for_eocd(std::istream& is) {
    EOCD s;

    ssize_t eof = is.tellg();
    if (eof < EOCD::SPEC_MIN_SIZE) {
        return std::unexpected("Not enough bytes for an EOCD");
    }

    for (ssize_t seekPos = -22; seekPos > (eof * -1); seekPos--) {
        is.seekg(seekPos, std::ios_base::end);
        is.read(reinterpret_cast<char*>(&s.signature), sizeof(s.signature));
        if (s.signature != 0x06054b50) {
            continue;
        }

        is.read(reinterpret_cast<char*>(&s.disk_number), sizeof(s.disk_number));
        is.read(reinterpret_cast<char*>(&s.num_disk_with_central_directory_start), sizeof(s.num_disk_with_central_directory_start));
        is.read(reinterpret_cast<char*>(&s.total_num_entries_central_directory_this_disk), sizeof(s.total_num_entries_central_directory_this_disk));
        is.read(reinterpret_cast<char*>(&s.total_num_entries_central_directory), sizeof(s.total_num_entries_central_directory));
        is.read(reinterpret_cast<char*>(&s.size_central_directory), sizeof(s.size_central_directory));
        is.read(reinterpret_cast<char*>(&s.offset_start_central_directory), sizeof(s.offset_start_central_directory));
        is.read(reinterpret_cast<char*>(&s.comment_length), sizeof(s.comment_length));

        if (eof - is.tellg() != s.comment_length) {
            continue;
        }

        s.comment.resize(s.comment_length);
        is.read(&s.comment[0], s.comment_length);
        return s;
    }

    return std::unexpected("Unable to find EOCD");
}
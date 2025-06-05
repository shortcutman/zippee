
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
    os << "Signature: 0x" << std::hex << s.signature;

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
        return std::unexpected("Not enough bytes for an EOCD");
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

    return std::unexpected("Unable to find EOCD");
}
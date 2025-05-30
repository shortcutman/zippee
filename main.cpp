
#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>

void println(const char* c) {
    std::cout << c << std::endl;
}

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

    bool good = false;

    friend std::ostream& operator<<(std::ostream& os, const EOCD& s) {
        os << "Signature: 0x" << std::hex << s.signature;

        if (s.comment.empty()) {
            os << " No comment";
        } else {
            os << " Comment: " << s.comment;
        }

        return os;
    }
};

int main(int argc, char** argv) {
    std::string filepath = "***REMOVED***";
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        println("file is not open");
        return -1;
    }

    if (file.tellg() < sizeof(EOCD)) {
        println("File smaller than EOCD.");
        return -1;
    }

    EOCD s;
    ssize_t eof = file.tellg();
    ssize_t seekPos = -22;
    file.seekg(seekPos, std::ios_base::end);
    file.read(reinterpret_cast<char*>(&s.signature), sizeof(s.signature));
    file.read(reinterpret_cast<char*>(&s.disk_number), sizeof(s.disk_number));
    file.read(reinterpret_cast<char*>(&s.num_disk_with_central_directory_start), sizeof(s.num_disk_with_central_directory_start));
    file.read(reinterpret_cast<char*>(&s.total_num_entries_central_directory_this_disk), sizeof(s.total_num_entries_central_directory_this_disk));
    file.read(reinterpret_cast<char*>(&s.total_num_entries_central_directory), sizeof(s.total_num_entries_central_directory));
    file.read(reinterpret_cast<char*>(&s.size_central_directory), sizeof(s.size_central_directory));
    file.read(reinterpret_cast<char*>(&s.offset_start_central_directory), sizeof(s.offset_start_central_directory));
    file.read(reinterpret_cast<char*>(&s.comment_length), sizeof(s.comment_length));

    if (s.signature != 0x06054b50) {
        println("No signature");
        return -1;
    }

    if (eof - file.tellg() != s.comment_length) {
        println("Comment length doesn't align");
        return -1;
    }

    s.comment.resize(s.comment_length);
    file.read(&s.comment[0], s.comment_length);

    std::cout << s << std::endl;

    return 1;
}

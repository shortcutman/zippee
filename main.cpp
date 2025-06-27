
#include "zip.hpp"
#include "deflate.hpp"


#include <cstdint>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <vector>

namespace {
    void writeout(const std::vector<std::byte>& d) {
        std::ofstream decompressed_file("outfile.txt", std::ios::binary);
        for (auto b : d) {
            decompressed_file.write(reinterpret_cast<char*>(&b), sizeof(b));
        }
    }
}

int main(int argc, char** argv) {
    std::string filepath = "***REMOVED***";
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::println("file is not open");
        return -1;
    }

    size_t size = file.tellg();
    
    std::vector<std::byte> data;
    data.reserve(size);
    file.seekg(0);

    for (size_t i = 0; i < size; i++) {
        auto getbyte = file.get();
        if (getbyte == std::ifstream::traits_type::eof()) {
            break;
        }

        data.push_back(std::byte{static_cast<uint8_t>(getbyte)});
    }

    auto result = zip::search_for_eocd(std::span{data.begin(), data.end()});
    std::cout << result.value() << std::endl;

    auto centralDir = std::span{data.begin() + result.value().offset_start_central_directory, data.end()};
    auto headers = zip::read_central_directory_headers(centralDir);
    for (auto& h : headers) {
        std::cout << h << std::endl;
    }

    auto actualData = std::span{data.begin(), data.end()};
    auto localHeader = zip::read_local_header(actualData);
    if (localHeader) {
        std::cout << "has local file header of size: " << localHeader.value().header_size() << std::endl;
    }

    auto decompressed = deflate::decompress(std::span{data.begin() + localHeader.value().header_size(), data.end()});

    writeout(decompressed);

    return 0;
}


#include "zip.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


void println(const char* c) {
    std::cout << c << std::endl;
}

int main(int argc, char** argv) {
    std::string filepath = "***REMOVED***";
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        println("file is not open");
        return -1;
    }

    auto size = file.tellg();
    std::cout << "size: " << size << std::endl;
    
    std::vector<std::byte> data;
    data.reserve(size);
    file.seekg(0);

    std::cout << "v size: " << data.size() << std::endl;

    for (size_t i; i < size; i++) {
        auto getbyte = file.get();
        if (getbyte == std::ifstream::traits_type::eof()) {
            break;
        }

        data.push_back(std::byte{static_cast<uint8_t>(getbyte)});
    }

    std::cout << "v size: " << data.size() << std::endl;

    auto result = zip::search_for_eocd(std::span{data.begin(), data.end()});
    std::cout << result.value() << std::endl;

    auto centralDir = std::span{data.begin() + result.value().offset_start_central_directory, data.end()};
    auto headers = zip::read_central_directory_headers(centralDir);
    for (auto& h : headers) {
        std::cout << h << std::endl;
    }

    return 0;
}

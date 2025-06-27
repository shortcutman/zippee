
#include "zip.hpp"
#include "deflate.hpp"

#include "vendor/CLI11.hpp"

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

    std::string input_filepath;
    bool list_files = false;

    CLI::App app{"zippee can decompress data contained with a ZIP file that is compressed with DEFLATE.", "zippee"};
    app.add_option("input", input_filepath, "Input file.")->required();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    std::ifstream input_file(input_filepath, std::ios::binary | std::ios::ate);

    if (!input_file.is_open()) {
        std::println("file is not open");
        return -1;
    }

    size_t size = input_file.tellg();
    
    std::vector<std::byte> data;
    data.reserve(size);
    input_file.seekg(0);

    for (size_t i = 0; i < size; i++) {
        auto getbyte = input_file.get();
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

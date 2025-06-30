
//------------------------------------------------------------------------------
// main.cpp
//------------------------------------------------------------------------------

#include "crc32.hpp"
#include "deflate.hpp"
#include "zip.hpp"

#include "vendor/CLI11.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <vector>

namespace {
    void writeout(const std::string& filename, const std::vector<std::byte>& d) {
        std::string adjusted = filename;
        std::replace(adjusted.begin(), adjusted.end(), '/', '_');

        std::ofstream decompressed_file(adjusted, std::ios::binary);
        decompressed_file.write(reinterpret_cast<char*>(const_cast<std::byte*>(&d[0])), d.size() * sizeof(std::byte));
    }
}

int main(int argc, char** argv) {
    std::string input_filepath;
    bool list_contents = false;

    CLI::App app{"zippee can decompress data contained with a ZIP file that is compressed with DEFLATE.", "zippee"};
    app.add_option("input", input_filepath, "Input file.")->required();
    app.add_flag("--list", list_contents, "List all contents of ZIP only.");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    std::ifstream input_file(input_filepath, std::ios::binary | std::ios::ate);

    if (!input_file.is_open()) {
        std::println("Unable to open file.");
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
    auto data_span = std::span{data};

    auto eocd = zip::search_for_eocd(data_span);
    auto centralDir = data_span.subspan(eocd.value().offset_start_central_directory, data_span.size() - eocd.value().offset_start_central_directory);
    auto headers = zip::read_central_directory_headers(centralDir);

    for (auto& h : headers) {
        std::println("Found {}.", h.file_name);

        auto local_header_data = data_span.subspan(h.relative_offset_of_local_header, data_span.size() - h.relative_offset_of_local_header);
        auto local_header = zip::read_local_header(local_header_data).value();

        if (!list_contents) {
            auto compressed_span = data_span.subspan(h.relative_offset_of_local_header + local_header.header_size(), data_span.size() - h.relative_offset_of_local_header - local_header.header_size());
            auto decompressed = deflate::decompress(compressed_span);

            auto crc32 = zip::crc32(decompressed);
            // below does not honour 4.4.4 of APPNOTE.TXT and check for data descriptor
            auto check_crc32 = local_header.crc_32 == 0 ? h.crc_32 : local_header.crc_32;

            if (check_crc32 != crc32) {
                std::println("CRC32 does not match for {}.", local_header.file_name);
            } else {
                writeout(local_header.file_name, decompressed);
                std::println("Decompressed and wrote out {}.", local_header.file_name);
            }            
        }
    }

    return 0;
}

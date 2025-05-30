
#include "zip.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>

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

    auto result = zip::search_for_eocd(file);
    std::cout << result.value() << std::endl;

    return 1;
}


//------------------------------------------------------------------------------
// deflate.cpp
//------------------------------------------------------------------------------

#include "deflate.hpp"

#include <utility>

std::vector<std::byte> deflate::decompress(std::span<std::byte> data) {
    uint8_t bit_offset = 0;   
    return std::vector<std::byte>();
}

bool deflate::is_bfinal(std::span<std::byte> data, uint8_t bit_offset) {

    auto flag_byte = data[0] >> bit_offset;

    if ((flag_byte & std::byte{0x01}) == std::byte{0x01}) {
        return true;
    }

    return false;
}

deflate::BType deflate::get_btype(std::span<std::byte> data, uint8_t bit_offset) {
    auto flag_byte = data[0] >> bit_offset;
    flag_byte &= std::byte{0x06};
    
    flag_byte >>= 1;
    return BType(std::to_underlying<std::byte>(flag_byte));
}

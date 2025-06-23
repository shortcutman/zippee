
//------------------------------------------------------------------------------
// bitspan.cpp
//------------------------------------------------------------------------------

#include "bitspan.hpp"

#include <cstddef>

zippee::bitspan::bitspan(std::span<std::byte> data)
    : _data(data)
    , _bit_offset(0) {
}

zippee::bitspan::bitspan(std::span<std::byte> data, size_t bit_offset)
    : _data(data)
    , _bit_offset(bit_offset) {
}

zippee::bitspan::bitspan(bitspan& data) 
    : _data(data._data) 
    , _bit_offset(data._bit_offset) {
}

size_t zippee::bitspan::bits_read() const {
    return _bit_offset;
}

uint32_t zippee::bitspan::peek_bits(uint8_t bits) {
    if (bits == 0) {
        return 0;
    }

    int64_t bits_remain = static_cast<int64_t>(_data.size() * 8) - _bit_offset - bits;
    if (bits_remain < 0) {
        throw std::runtime_error("Not enough bits available.");
    }

    size_t byte_offset = _bit_offset / 8;
    size_t bits_in = _bit_offset % 8;

    uint32_t val = *reinterpret_cast<uint32_t*>(&_data[byte_offset]);
    val >>= bits_in;
    //mask for bits asked for
    val &= 0xffffffff >> (32 - bits);

    return val;
}

uint32_t zippee::bitspan::read_bits(uint8_t bits) {
    auto ret = peek_bits(bits);
    _bit_offset += bits;
    return ret;
}
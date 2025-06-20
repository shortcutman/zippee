
//------------------------------------------------------------------------------
// bitspan.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <span>

namespace zippee {
    class bitspan {
    private:
        std::span<std::byte> _data;
        size_t _bit_offset;

    public:
        bitspan(std::span<std::byte> data);
        bitspan(std::span<std::byte> data, size_t bit_offset);

        size_t bits_read() const;

        uint32_t peek_bits(uint8_t bits);
        uint32_t read_bits(uint8_t bits);
    };
}

//------------------------------------------------------------------------------
// crc32.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <vector>

namespace zip {
    uint32_t crc32(const std::vector<std::byte>& data);
}

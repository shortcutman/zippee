
//------------------------------------------------------------------------------
// crc32.tests.cpp
//------------------------------------------------------------------------------

#include "crc32.hpp"

#include <gtest/gtest.h>

namespace {

// https://stackoverflow.com/a/45172360
template<typename... Ts>
std::array<std::byte, sizeof...(Ts)> make_bytes(Ts&&... args) noexcept {
    return{std::byte(std::forward<Ts>(args))...};
}

}

TEST(CRC32, basic) {
    auto data = std::vector<std::byte>({
        std::byte{'H'}, std::byte{'i'}, std::byte{'\n'}
    });

    auto crc32 = zip::crc32(data);

    EXPECT_EQ(crc32, 0xd5223c9a);
}

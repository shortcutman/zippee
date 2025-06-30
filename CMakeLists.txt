cmake_minimum_required(VERSION 3.11)

project(zippee)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_COMPILE_WARNING_AS_ERROR True)

add_compile_options(-Wall)

add_library(zip
    bitspan.cpp
    crc32.cpp
    deflate.cpp
    zip.cpp
)

add_executable(zippee
    main.cpp
)

target_link_libraries(
    zippee
    zip
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    zip_tests
    bitspan.tests.cpp
    crc32.tests.cpp
    deflate.tests.cpp
    zip.tests.cpp
)
target_link_libraries(
    zip_tests
    zip
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(zip_tests)
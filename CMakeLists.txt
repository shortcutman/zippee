cmake_minimum_required(VERSION 3.11)

project(zippee)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_COMPILE_WARNING_AS_ERROR True)

add_executable(zippee
    main.cpp
    zip.cpp
)

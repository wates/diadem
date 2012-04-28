cmake_minimum_required(VERSION 2.6)

project(jpncode)

add_library(jpncode
    jpncode.cc
    jpncode.h
)

add_executable(test_moji
    test_moji.cc
    moji.h
    moji.cc
)

target_link_libraries(test_moji jpncode gtest)


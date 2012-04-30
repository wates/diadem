cmake_minimum_required(VERSION 2.6)

project(diadem)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "-Wall -fno-strict-aliasing")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    add_definitions(-DGCC)
elseif(MSVC)
    set(CMAKE_C_FLAGS "/W4 /wd4819")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    add_definitions(-DWINDOWS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_library(gtest gtest/gtest.h gtest/gtest_main.cc gtest/gtest-all.cc)
if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(gtest pthread)
endif()


add_subdirectory(jpncode)
add_subdirectory(wts)
add_subdirectory(diadem)


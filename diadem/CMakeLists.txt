cmake_minimum_required(VERSION 2.6)

project(diadem)

add_executable(diadem
    diadem.cc
    storage.cc
    storage_mysql.cc
    diadem.h
    jsonconv.h
    storage.h
)

target_link_libraries(diadem jpncode wts)

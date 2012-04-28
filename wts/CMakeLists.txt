cmake_minimum_required(VERSION 2.6)

project(wts)

add_library(wts
    allocator.cc
    pipe.cc
    pipe_tcp.cc
    system.cc
    system_linux.cc
    system_windows.cc

    wts_types.h
    
    allocator.h
    container.h
    meta_container.h
    converter.h
    parser.h
    system.h
    pipe.h
    pipe_tcp.h
    observer.cc
    observer.h
)


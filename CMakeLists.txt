cmake_minimum_required(VERSION 3.28)
project(quark)

include_directories(
    ./libs
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(quark
    main.cpp
    qrcodegen.cpp
)

cmake_minimum_required(VERSION 3.15)
project(pawnshop_core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(pybind11 CONFIG REQUIRED)
include_directories(include)

pybind11_add_module(pawnshop_core src/bindings.cpp)
add_executable(pawnshop_cli src/main.cpp)   # твій поточний main.cpp
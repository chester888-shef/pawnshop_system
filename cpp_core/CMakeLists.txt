cmake_minimum_required(VERSION 3.15)
project(lombard_core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PYBIND11_FINDPYTHON ON)
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

add_library(lombard_core STATIC src/pledge.cpp)
set_target_properties(lombard_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(lombard_core PUBLIC include)
pybind11_add_module(pawnshop_core src/bindings.cpp)
target_link_libraries(pawnshop_core PRIVATE lombard_core)
add_executable(pawnshop_cli src/main.cpp)
target_link_libraries(pawnshop_cli PRIVATE lombard_core)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME domain_integration
        COMMAND ${Python_EXECUTABLE} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR}/../tests -v)
    set_tests_properties(domain_integration PROPERTIES
        ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:pawnshop_core>:${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()

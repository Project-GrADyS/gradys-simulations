cmake_minimum_required(VERSION 3.5...3.26)
project(simulation LANGUAGES CXX)

add_subdirectory(extern/pybind11)

add_executable(simulation main.cc)
target_link_libraries(simulation PRIVATE pybind11::embed)

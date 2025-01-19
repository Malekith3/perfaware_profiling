cmake_minimum_required(VERSION 3.28)
project(perfaware_profiling)

set(CMAKE_CXX_STANDARD 17)

add_executable(haversine_generator
        haversine_formula.cpp
        haversine_generator.cpp)
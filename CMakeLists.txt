cmake_minimum_required(VERSION 3.28)
project(perfaware_profiling)

set(CMAKE_CXX_STANDARD 17)

add_executable(haversine_generator
        external/haversine_formula.cpp
        HarversineCoordGenerator/haversine_generator.cpp)

include_directories(${CMAKE_SOURCE_DIR}/external/)

include(CTest)
#TestDefines
add_executable(test_json_parser
        JSONParser/json_parser.cpp
        test/test_json_parser.cpp)

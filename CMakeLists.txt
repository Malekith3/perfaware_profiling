cmake_minimum_required(VERSION 3.28)
project(perfaware_profiling)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROFILER=1")
include_directories(${CMAKE_SOURCE_DIR}/external/)
include_directories(${CMAKE_SOURCE_DIR}/JSONParser/)
include_directories(${CMAKE_SOURCE_DIR}/HaversineCLIApp/)
include_directories(${CMAKE_SOURCE_DIR}/profiling_assembly/)

add_executable(test_json_parser
        JSONParser/json_parser.cpp
        JSONParser/test/test_json_parser.cpp)

add_executable(haversine_cli_app
        external/haversine_formula.cpp
        HaversineCLIApp/haversine_cli_app.cpp
        JSONParser/json_parser.cpp)

add_executable(haversine_generator
        external/haversine_formula.cpp
        HaversineCoordGenerator/haversine_generator.cpp)



set(ASM_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/profiling_assembly")

add_executable(assembly_loop_profiling
        profiling_assembly/listing_0133_front_end_test_main.cpp)

target_link_directories(assembly_loop_profiling PRIVATE ${ASM_LIBRARY_PATH})
target_link_libraries(assembly_loop_profiling PRIVATE listing_0132_nop_loop)
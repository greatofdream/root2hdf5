cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(JPDataConverter)
set(CMAKE_CXX_STANDARD 17) # required for argparse

find_package(ROOT)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

find_package(HDF5 COMPONENTS CXX HL)
include_directories(${HDF5_INCLUDE_DIRS})

add_executable(ConvertRawData ConvertRawData.C)
target_link_libraries(ConvertRawData ${ROOT_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(JPDataConverter)
set(CMAKE_CXX_STANDARD 17) # required for argparse

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

find_package(ROOT)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

find_package(HDF5 COMPONENTS CXX HL)
include_directories(${HDF5_INCLUDE_DIRS})

add_library(TransformReadoutTree SHARED TransformReadoutTree.C)
target_link_libraries(TransformReadoutTree ${ROOT_LIBRARIES} ${HDF5_LIBRARIES}
    ${HDF5_HL_LIBRARIES})
add_library(TransformRunHeaderTree SHARED TransformRunHeaderTree.C)
target_link_libraries(TransformRunHeaderTree ${ROOT_LIBRARIES} ${HDF5_LIBRARIES}
    ${HDF5_HL_LIBRARIES})
add_library(TransformTrack SHARED ./TransformTrack.C)
target_link_libraries(TransformTrack ${ROOT_LIBRARIES}
    ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
add_library(TransformSimTriggerInfoTree SHARED TransformSimTriggerInfoTree.C)
target_link_libraries(TransformSimTriggerInfoTree ${ROOT_LIBRARIES}
    ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
add_library(TransformSimTruthTree SHARED TransformSimTruthTree.C)
target_link_libraries(TransformSimTruthTree TransformTrack ${ROOT_LIBRARIES}
    ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

add_executable(ConvertRawData ConvertRawData.C)
target_link_libraries(ConvertRawData TransformReadoutTree ${ROOT_LIBRARIES}
    ${HDF5_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

add_executable(ConvertSimData ConvertSimData.C)
set(JSAPSYS $ENV{JSAPSYS})
target_include_directories(ConvertSimData PRIVATE "${JSAPSYS}/DataType/include")
target_link_libraries(ConvertSimData ${ROOT_LIBRARIES} ${HDF5_LIBRARIES}
    ${HDF5_HL_LIBRARIES} "${JSAPSYS}/DataType/lib/libJPSIMOUTPUT.so"
    TransformReadoutTree TransformRunHeaderTree
    TransformSimTriggerInfoTree TransformSimTruthTree)
configure_file("${JSAPSYS}/DataType/lib/libJPSIMOUTPUT_rdict.pcm"
    ${PROJECT_BINARY_DIR}/ COPYONLY)

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(JPDataConverter)
set(CMAKE_CXX_STANDARD 17) # required for argparse

set(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

find_package(ROOT)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

find_package(HDF5 COMPONENTS CXX HL)
include_directories(${HDF5_INCLUDE_DIRS})

set(JSAPSYS $ENV{JSAPSYS})
include_directories("${JSAPSYS}/Simulation/DataType/include")


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
target_link_libraries(ConvertSimData ${ROOT_LIBRARIES} ${HDF5_LIBRARIES}
	${HDF5_HL_LIBRARIES} TransformReadoutTree TransformRunHeaderTree
    TransformSimTriggerInfoTree TransformSimTruthTree)

add_executable(ConvertGH1 GhostHunter1.C TransformPE.C)
target_link_libraries(ConvertGH1 TransformReadoutTree ${ROOT_LIBRARIES}
    ${HDF5_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

configure_file("${JSAPSYS}/Simulation/DataType/lib/libJPSIMOUTPUT_rdict.pcm" ${PROJECT_BINARY_DIR}/ COPYONLY)
configure_file("${JSAPSYS}/Simulation/DataType/lib/libJPSIMOUTPUT.so" ${PROJECT_BINARY_DIR}/ COPYONLY)
install(TARGETS ConvertRawData ConvertSimData RUNTIME DESTINATION bin)
install(TARGETS TransformReadoutTree TransformRunHeaderTree 
	TransformTrack TransformSimTruthTree TransformSimTriggerInfoTree RUNTIME
	DESTINATION lib)
install(FILE libJPSIMOUTPUT.so RUNTIME DESTINATION bin)
install(FILE libJPSIMOUTPUT_rdict.pcm RUNTIME DESTINATION bin)

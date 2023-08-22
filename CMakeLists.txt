cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(READSKROOTProj CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

find_package(ROOT REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX HL)
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
add_executable(root2hdf5 root2hdf5.cpp converter.cpp)
target_include_directories(root2hdf5 PUBLIC ${ROOT_INCLUDE_DIRS}
	PUBLIC $ENV{SKOFL_ROOT}/include
	PUBLIC ${HDF5_INCLUDE_DIRS}
	)
target_link_libraries(root2hdf5 PRIVATE ${ROOT_LIBRARIES}
	PRIVATE ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES}
	PRIVATE $ENV{SKOFL_ROOT}/lib/libtqrealroot.so $ENV{SKOFL_ROOT}/lib/libloweroot.so
	)
target_compile_definitions(root2hdf5
                           PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})


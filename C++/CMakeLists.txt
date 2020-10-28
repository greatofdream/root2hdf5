cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(H5Test)

set(CMAKE_INSTALL_PREFIX ${JSAPSYS}/Analysis/DataQuality)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

find_package(HDF5 COMPONENTS CXX HL)
include_directories(${HDF5_INCLUDE_DIRS})

message("HAHAAHA ${HDF5_LIBRARIES}")
add_executable(TestH5 main.C)
target_link_libraries(TestH5 ${ROOT_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

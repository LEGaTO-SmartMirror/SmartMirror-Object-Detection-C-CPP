cmake_minimum_required(VERSION 3.5.1)

# Define variables:

# Project name.
set(PROJ_NAME object_detection)
# Path to Darknet project folder.
set(DARKNET_PATH "/opt/dev/dependencies/darknet/")
# Give a custom name to shared library which is provided by Darkned when the one compiled.
set(DARKNET_LIB_NAME libdarknet)
# Define C++ level, could be 11 or 17 as well.
set(CMAKE_CXX_STANDARD 14)
# Define compiler optimization level.
set(CMAKE_CXX_FLAGS "-O3")
# Do print warnings uppon compilation, let's keep our code as clean as possible.
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -Wall -Wextra -g")
# Apply flags.
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -pthread")  #force pthread to avoid bugs in some cmake setups
set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -pthread -g")

set(CMAKE_BUILD_TYPE Debug)

project(${PROJ_NAME})

find_package(OpenCV)
find_package(CUDA REQUIRED)

# Note: We do not recommend using GLOB or GLOB_RECURSE to collect a list of source files from your source tree.
# If no CMakeLists.txt file changes when a source is added or removed then the generated build system cannot know
# when to ask CMake to regenerate.
file(GLOB_RECURSE SOURCE_FILES "src/*")

add_executable(${PROJ_NAME} ${SOURCE_FILES})

# Link Darknet library' include folder.
include_directories(${DARKNET_PATH}/include)
include_directories(${DARKNET_PATH}/src)
include_directories(/usr/local/cuda/include/)

# Define Darknet library' shared library.
add_library(${DARKNET_LIB_NAME} SHARED IMPORTED)
# Set a path to Darknet library' shared library
set_property(TARGET ${DARKNET_LIB_NAME} PROPERTY IMPORTED_LOCATION "${DARKNET_PATH}/libdarknet.so")

# Link all libraries together.
target_link_libraries(${PROJ_NAME} ${DARKNET_LIB_NAME} ${OpenCV_LIBRARIES} cuda cudart)

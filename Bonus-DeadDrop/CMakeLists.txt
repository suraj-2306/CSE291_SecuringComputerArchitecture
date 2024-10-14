# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(CovertChannel C)

# Set the C standard version to C99
set(CMAKE_C_STANDARD 99)

# Enable debug symbols and disable optimizations in Debug mode
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

# List of shared source files
set(SHARED_SOURCES util.c)

# Include directories (add the root directory where util.h resides)
include_directories(${CMAKE_SOURCE_DIR})

# Create sender target
add_executable(sender sender.c ${SHARED_SOURCES})

# Create receiver target
add_executable(receiver receiver.c ${SHARED_SOURCES})

# Set default build type to Debug if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

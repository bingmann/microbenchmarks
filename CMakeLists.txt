################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

cmake_minimum_required(VERSION 3.9.2)
cmake_policy(VERSION 3.9.2)

project(microbenchmarks)

# prohibit in-source builds
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif()

# Set a default build type if none was specified
set(MBM_DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${MBM_DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${MBM_DEFAULT_BUILD_TYPE}" CACHE
    STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

################################################################################
### Options and Switches

################################################################################
### Compiler Flags

# variables to collect compile-time definitions, include dirs, and libraries
set(MBM_DEFINITIONS "")
set(MBM_INCLUDE_DIRS "")
set(MBM_LINK_LIBRARIES "")

# enable more warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")

# use C++17
set(CMAKE_CXX_STANDARD 17)

# enable warnings
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -march=native")

# with run-time STL checks
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG")

# with AddressSanitizer
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

# enable ThreadSanitizer
if(OFF)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -pie -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBM_HAVE_THREAD_SANITIZER=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -pie -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMBM_HAVE_THREAD_SANITIZER=1")
endif()

message(STATUS "MBM CMAKE_CXX_FLAGS:" ${CMAKE_CXX_FLAGS})

################################################################################
### Find Required Libraries

### find pthreads ###

find_package(Threads REQUIRED)
set(MBM_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${MBM_LINK_LIBRARIES})
if(CMAKE_USE_PTHREADS_INIT)
  set(MBM_LINK_LIBRARIES pthread ${MBM_LINK_LIBRARIES})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

### use Boost ###

find_package(Boost 1.42.0 COMPONENTS container)

if(${Boost_FOUND})
  include_directories(${Boost_INCLUDE_DIRS})
  set(MBM_LINK_LIBRARIES ${Boost_LIBRARIES} ${MBM_LINK_LIBRARIES})
endif()

### use TLX ###

add_subdirectory(extlib/tlx)
set(MBM_LINK_LIBRARIES tlx ${MBM_LINK_LIBRARIES})

################################################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(ordered_sets)
add_subdirectory(sort)

add_executable(results_to_tsv results_to_tsv.cpp)

################################################################################

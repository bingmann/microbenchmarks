################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../extlib/abseil-cpp)

add_executable(mbm_ordered_sets mbm_ordered_sets.cpp)
target_link_libraries(mbm_ordered_sets ${MBM_LINK_LIBRARIES})

################################################################################

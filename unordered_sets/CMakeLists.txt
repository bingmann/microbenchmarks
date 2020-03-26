################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sparsehash/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sparsepp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hopscotch-map/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/robin-map/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/robin-hood-hashing/src/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libcuckoo/libcuckoo)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../extlib/abseil-cpp)

add_executable(mbm_unordered_sets mbm_unordered_sets.cpp)
target_link_libraries(mbm_unordered_sets ${MBM_LINK_LIBRARIES})
target_link_libraries(mbm_unordered_sets absl::flat_hash_map absl::node_hash_map)

################################################################################

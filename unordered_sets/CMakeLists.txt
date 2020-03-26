################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/sparsehash/src)
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/sparsepp)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/hopscotch-map/include)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/robin-map/include)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/robin-hood-hashing/src/include)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/libcuckoo/libcuckoo)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../extlib/abseil-cpp)

set(PROGRAM_LIST
  std_unordered_multiset
  google_sparse_hash_set
  google_dense_hash_set
  spp_sparse_hash_set
  tsl_hopscotch_set
  tsl_robin_set
  robin_hood_unordered_set
  absl_flat_hash_set2
  absl_node_hash_set2

  std_unordered_multimap
  google_sparse_hash_map
  google_dense_hash_map
  spp_sparse_hash_map
  tsl_hopscotch_map
  tsl_robin_map
  robin_hood_unordered_map
  libcuckoo_cuckoohash_map
  absl_flat_hash_map2
  absl_node_hash_map2
  )

foreach(F ${PROGRAM_LIST})

  add_executable(${F} mbm_unordered_sets.cpp)
  target_link_libraries(${F} ${MBM_LINK_LIBRARIES})

endforeach()

# select set algorithms
target_compile_definitions(std_unordered_multiset PRIVATE "MBM_SET_ALGORITHM=1")
target_compile_definitions(google_sparse_hash_set PRIVATE "MBM_SET_ALGORITHM=2")
target_compile_definitions(google_dense_hash_set PRIVATE "MBM_SET_ALGORITHM=3")
target_compile_definitions(spp_sparse_hash_set PRIVATE "MBM_SET_ALGORITHM=4")
target_compile_definitions(tsl_hopscotch_set PRIVATE "MBM_SET_ALGORITHM=5")
target_compile_definitions(tsl_robin_set PRIVATE "MBM_SET_ALGORITHM=6")
target_compile_definitions(robin_hood_unordered_set PRIVATE "MBM_SET_ALGORITHM=7")
target_compile_definitions(absl_flat_hash_set2 PRIVATE "MBM_SET_ALGORITHM=10")
target_compile_definitions(absl_node_hash_set2 PRIVATE "MBM_SET_ALGORITHM=11")

# select map algorithms
target_compile_definitions(std_unordered_multimap PRIVATE "MBM_MAP_ALGORITHM=1")
target_compile_definitions(google_sparse_hash_map PRIVATE "MBM_MAP_ALGORITHM=2")
target_compile_definitions(google_dense_hash_map PRIVATE "MBM_MAP_ALGORITHM=3")
target_compile_definitions(spp_sparse_hash_map PRIVATE "MBM_MAP_ALGORITHM=4")
target_compile_definitions(tsl_hopscotch_map PRIVATE "MBM_MAP_ALGORITHM=5")
target_compile_definitions(tsl_robin_map PRIVATE "MBM_MAP_ALGORITHM=6")
target_compile_definitions(robin_hood_unordered_map PRIVATE "MBM_MAP_ALGORITHM=7")
target_compile_definitions(libcuckoo_cuckoohash_map PRIVATE "MBM_MAP_ALGORITHM=8")
target_compile_definitions(absl_flat_hash_map2 PRIVATE "MBM_MAP_ALGORITHM=10")
target_compile_definitions(absl_node_hash_map2 PRIVATE "MBM_MAP_ALGORITHM=11")

# missing libraries
target_link_libraries(absl_flat_hash_set2 absl::flat_hash_map)
target_link_libraries(absl_flat_hash_map2 absl::flat_hash_map)
target_link_libraries(absl_node_hash_set2 absl::node_hash_map)
target_link_libraries(absl_node_hash_map2 absl::node_hash_map)

# write batch file
WriteRunAllBatch("${CMAKE_CURRENT_BINARY_DIR}/run_all.sh" "${PROGRAM_LIST}")

# fix google sparsehash
file(COPY sparseconfig.h
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sparsehash/internal/)

################################################################################

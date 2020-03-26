################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../extlib/abseil-cpp)

set(PROGRAM_LIST
  std_multiset std_unordered_multiset2 tlx_splay_multiset
  tlx_btree_multiset_004 tlx_btree_multiset_008 tlx_btree_multiset_016
  tlx_btree_multiset_032 tlx_btree_multiset_064 tlx_btree_multiset_128
  tlx_btree_multiset_256
  boost_flat_multiset google_btree_set absl_btree_set

  std_multimap std_unordered_multimap2
  tlx_btree_multimap_004 tlx_btree_multimap_008 tlx_btree_multimap_016
  tlx_btree_multimap_032 tlx_btree_multimap_064 tlx_btree_multimap_128
  tlx_btree_multimap_256
  boost_flat_map google_btree_map absl_btree_map
  )

foreach(F ${PROGRAM_LIST})

  add_executable(${F} mbm_ordered_sets.cpp)
  target_link_libraries(${F} ${MBM_LINK_LIBRARIES})

endforeach()

# select set algorithms
target_compile_definitions(std_multiset PRIVATE "MBM_SET_ALGORITHM=1")
target_compile_definitions(std_unordered_multiset2 PRIVATE "MBM_SET_ALGORITHM=2")
target_compile_definitions(tlx_splay_multiset PRIVATE "MBM_SET_ALGORITHM=3")

target_compile_definitions(tlx_btree_multiset_004 PRIVATE "MBM_SET_ALGORITHM=10")
target_compile_definitions(tlx_btree_multiset_008 PRIVATE "MBM_SET_ALGORITHM=11")
target_compile_definitions(tlx_btree_multiset_016 PRIVATE "MBM_SET_ALGORITHM=12")
target_compile_definitions(tlx_btree_multiset_032 PRIVATE "MBM_SET_ALGORITHM=13")
target_compile_definitions(tlx_btree_multiset_064 PRIVATE "MBM_SET_ALGORITHM=14")
target_compile_definitions(tlx_btree_multiset_128 PRIVATE "MBM_SET_ALGORITHM=15")
target_compile_definitions(tlx_btree_multiset_256 PRIVATE "MBM_SET_ALGORITHM=16")

target_compile_definitions(boost_flat_multiset PRIVATE "MBM_SET_ALGORITHM=20")
target_compile_definitions(google_btree_set PRIVATE "MBM_SET_ALGORITHM=21")
target_compile_definitions(absl_btree_set PRIVATE "MBM_SET_ALGORITHM=22")

# select map algorithms
target_compile_definitions(std_multimap PRIVATE "MBM_MAP_ALGORITHM=1")
target_compile_definitions(std_unordered_multimap2 PRIVATE "MBM_MAP_ALGORITHM=2")

target_compile_definitions(tlx_btree_multimap_004 PRIVATE "MBM_MAP_ALGORITHM=10")
target_compile_definitions(tlx_btree_multimap_008 PRIVATE "MBM_MAP_ALGORITHM=11")
target_compile_definitions(tlx_btree_multimap_016 PRIVATE "MBM_MAP_ALGORITHM=12")
target_compile_definitions(tlx_btree_multimap_032 PRIVATE "MBM_MAP_ALGORITHM=13")
target_compile_definitions(tlx_btree_multimap_064 PRIVATE "MBM_MAP_ALGORITHM=14")
target_compile_definitions(tlx_btree_multimap_128 PRIVATE "MBM_MAP_ALGORITHM=15")
target_compile_definitions(tlx_btree_multimap_256 PRIVATE "MBM_MAP_ALGORITHM=16")

target_compile_definitions(boost_flat_map PRIVATE "MBM_MAP_ALGORITHM=20")
target_compile_definitions(google_btree_map PRIVATE "MBM_MAP_ALGORITHM=21")
target_compile_definitions(absl_btree_map PRIVATE "MBM_MAP_ALGORITHM=22")

# write batch file
WriteRunAllBatch("${CMAKE_CURRENT_BINARY_DIR}/run_all.sh" "${PROGRAM_LIST}")

################################################################################

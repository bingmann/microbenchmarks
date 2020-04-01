################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

include_directories(${PROJECT_SOURCE_DIR}/sort)

set(PROGRAM_LIST
  ips4o_parallel_sort
  mcstl_parallel_mergesort parallel_msd_radix_sort parallel_lsd_radix_sort
  tbb_parallel_sort
  )

foreach(F ${PROGRAM_LIST})

  add_executable(${F} mbm_sort_parallel.cpp)
  target_link_libraries(${F} ${MBM_LINK_LIBRARIES} atomic TBB::tbb)

endforeach()

# select algorithms
target_compile_definitions(ips4o_parallel_sort
  PRIVATE "MBM_ALGORITHM=IPS4oParallelSort")
target_compile_definitions(mcstl_parallel_mergesort
  PRIVATE "MBM_ALGORITHM=MCSTLParallelMergesort")
target_compile_definitions(tbb_parallel_sort
  PRIVATE "MBM_ALGORITHM=TBBParallelSort")
target_compile_definitions(mcstl_parallel_mergesort
  PRIVATE "MBM_ALGORITHM=MCSTLParallelMergesort")
target_compile_definitions(parallel_msd_radix_sort
  PRIVATE "MBM_ALGORITHM=ParallelMSDRadixSort")
target_compile_definitions(parallel_lsd_radix_sort
  PRIVATE "MBM_ALGORITHM=ParallelLSDRadixSort")

# write batch file
WriteRunAllBatch("${CMAKE_CURRENT_BINARY_DIR}/run_all.sh" "${PROGRAM_LIST}")

################################################################################

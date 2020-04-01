################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

set(PROGRAM_LIST
  std_sort std_stable_sort ips4o_sequential_sort
  )

foreach(F ${PROGRAM_LIST})

  add_executable(${F} mbm_sort.cpp)
  target_link_libraries(${F} ${MBM_LINK_LIBRARIES} atomic)

endforeach()

# select algorithms
target_compile_definitions(std_sort
  PRIVATE "MBM_ALGORITHM=StdSort")
target_compile_definitions(std_stable_sort
  PRIVATE "MBM_ALGORITHM=StdStableSort")
target_compile_definitions(ips4o_sequential_sort
  PRIVATE "MBM_ALGORITHM=IPS4oSequentialSort")

# write batch file
WriteRunAllBatch("${CMAKE_CURRENT_BINARY_DIR}/run_all.sh" "${PROGRAM_LIST}")

################################################################################

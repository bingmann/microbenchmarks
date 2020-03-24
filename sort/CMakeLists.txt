################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

add_executable(mbm_sort mbm_sort.cpp)
target_link_libraries(mbm_sort ${MBM_LINK_LIBRARIES})

################################################################################

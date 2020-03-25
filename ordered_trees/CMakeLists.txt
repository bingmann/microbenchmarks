################################################################################
# CMakeLists.txt
#
# Copyright (c) 2020 Timo Bingmann
#
# All rights reserved. Published under the MIT License in the LICENSE file.
################################################################################

add_executable(mbm_trees mbm_trees.cpp)
target_link_libraries(mbm_trees ${MBM_LINK_LIBRARIES})

################################################################################

# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)

# project and version
project (jome VERSION 0.1.0)

# configure compiler globally for C++14
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# data build path
set (JOME-DATA-DIR "${CMAKE_CURRENT_BINARY_DIR}/data")

# jome
add_subdirectory (assets)
add_subdirectory (gen-data)
add_subdirectory (jome)

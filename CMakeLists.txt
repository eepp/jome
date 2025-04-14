# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

cmake_minimum_required (VERSION 3.30.0 FATAL_ERROR)

# project and version
project (jome VERSION 1.3.0)

# configure compiler globally for C++14
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# jome
add_subdirectory (assets)
add_subdirectory (man)
add_subdirectory (jome)
add_subdirectory (jome-ctl)

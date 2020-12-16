# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

# Qt5
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
find_package (Qt5Network CONFIG REQUIRED)

# jome-ctl program
add_executable (
    jome-ctl
    jome-ctl.cpp
    q-ctl-client.cpp
)
target_link_libraries (
    jome-ctl
    Qt5::Network
)
target_compile_definitions (
    jome-ctl PRIVATE
    "-DJOME_VERSION=\"${PROJECT_VERSION}\""
)
target_compile_options (
    jome-ctl PRIVATE
    -Wall -Wextra -Wno-deprecated-declarations
)
install (
    TARGETS jome-ctl
    RUNTIME DESTINATION bin
)

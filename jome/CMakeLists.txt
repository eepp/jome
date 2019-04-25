# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

# Qt5
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
find_package (Qt5Widgets CONFIG REQUIRED)
find_package (Qt5Gui CONFIG REQUIRED)
find_package (Qt5Network CONFIG REQUIRED)

# Boost
find_package (Boost 1.58 REQUIRED)

# jome program
add_executable (
    jome
    jome.cpp
    q-jome-window.cpp
    q-cat-list-widget-item.cpp
    q-emoji-graphics-item.cpp
    q-emojis-widget.cpp
    q-jome-server.cpp
    emoji-images.cpp
    emoji-db.cpp
    tinyutf8.cpp
)
add_dependencies (jome data)
target_link_libraries (
    jome
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network
)
target_include_directories (
    jome PRIVATE
    ${Boost_INCLUDE_DIRS}
)
target_compile_definitions (
    jome PRIVATE
    "-DJOME_VERSION=\"${PROJECT_VERSION}\""
    "-DJOME_DATA_DIR=\"${CMAKE_INSTALL_PREFIX}/share/jome/data\""
)
install (
    TARGETS jome
    RUNTIME DESTINATION bin
)

# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

# Qt 5
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
find_package (Qt5Widgets CONFIG REQUIRED)
find_package (Qt5Gui CONFIG REQUIRED)
find_package (Qt5Network CONFIG REQUIRED)

# Boost
find_package (Boost 1.70 REQUIRED)

# JSON for Modern C++
include (FetchContent)
FetchContent_Declare (
	nlohmann_json
	URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable (nlohmann_json)

# {fmt}
set (FMT_INSTALL OFF)
FetchContent_Declare (
	fmt URL
	https://github.com/fmtlib/fmt/releases/download/11.1.4/fmt-11.1.4.zip
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable (fmt)

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
    settings.cpp
    emojipedia.cpp
)
target_link_libraries (
    jome
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network
    Boost::boost
    nlohmann_json::nlohmann_json
    fmt::fmt
)
target_include_directories (
    jome PRIVATE
)
target_compile_definitions (
    jome PRIVATE
    "-DJOME_VERSION=\"${PROJECT_VERSION}\""
    "-DJOME_DATA_DIR=\"${CMAKE_INSTALL_PREFIX}/share/jome/data\""
)
target_compile_options (
    jome PRIVATE
    -Wall -Wextra -Wno-deprecated-declarations
)
install (
    TARGETS jome
    RUNTIME DESTINATION bin
)

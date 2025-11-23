cmake_minimum_required(VERSION 3.16)
project(obs-youtube-superchat-plugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OBS Studio
find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)

# Find Qt (try Qt6 first, fallback to Qt5)
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
if(Qt6_FOUND)
    message(STATUS "Using Qt6")
    set(QT_VERSION 6)
else()
    message(STATUS "Qt6 not found, trying Qt5...")
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
    message(STATUS "Using Qt5")
    set(QT_VERSION 5)
endif()

# Find cURL for HTTP requests
find_package(CURL REQUIRED)

# Find nlohmann_json for JSON parsing
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Plugin sources
set(PLUGIN_SOURCES
    src/plugin-main.cpp
    src/youtube-chat-client.cpp
    src/obstruction-manager.cpp
    src/settings-dialog.cpp
)

set(PLUGIN_HEADERS
    src/plugin-main.hpp
    src/youtube-chat-client.hpp
    src/obstruction-manager.hpp
    src/settings-dialog.hpp
)

# Create plugin library
add_library(obs-youtube-superchat-plugin MODULE
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# Include directories
target_include_directories(obs-youtube-superchat-plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
if(QT_VERSION EQUAL 6)
    target_link_libraries(obs-youtube-superchat-plugin
        OBS::libobs
        OBS::obs-frontend-api
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        CURL::libcurl
        nlohmann_json::nlohmann_json
    )
else()
    target_link_libraries(obs-youtube-superchat-plugin
        OBS::libobs
        OBS::obs-frontend-api
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        CURL::libcurl
        nlohmann_json::nlohmann_json
    )
endif()

# Set install directory
if(WIN32)
    set(PLUGIN_INSTALL_DIR "obs-plugins/64bit")
    set(DATA_INSTALL_DIR "data/obs-plugins/obs-youtube-superchat-plugin")
elseif(APPLE)
    set(PLUGIN_INSTALL_DIR "Library/Application Support/obs-studio/plugins")
    set(DATA_INSTALL_DIR "Library/Application Support/obs-studio/plugins/obs-youtube-superchat-plugin/data")
else()
    set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/obs-plugins")
    set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/obs-youtube-superchat-plugin")
endif()

# Install plugin
install(TARGETS obs-youtube-superchat-plugin
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
)

# Install data files
install(DIRECTORY data/
    DESTINATION ${DATA_INSTALL_DIR}
)

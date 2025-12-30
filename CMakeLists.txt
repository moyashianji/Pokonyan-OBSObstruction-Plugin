cmake_minimum_required(VERSION 3.16)

# Visual Studio 2022 Build Tools configuration
# Standard library headers are provided by the MSVC toolchain

project(obs-youtube-superchat-plugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC (Meta-Object Compiler) processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find OBS Studio
# First check if we're using an OBS build directory
set(OBS_BUILD_DIR_DETECTED FALSE)
foreach(prefix_path ${CMAKE_PREFIX_PATH})
    if(EXISTS "${prefix_path}/libobs/Release/obs.lib")
        set(OBS_BUILD_DIR "${prefix_path}")
        set(OBS_BUILD_DIR_DETECTED TRUE)
        message(STATUS "Detected OBS build directory: ${OBS_BUILD_DIR}")
        break()
    endif()
endforeach()

if(OBS_BUILD_DIR_DETECTED)
    # Use OBS from build directory directly
    add_library(OBS::libobs SHARED IMPORTED)
    set_target_properties(OBS::libobs PROPERTIES
        IMPORTED_LOCATION "${OBS_BUILD_DIR}/libobs/Release/obs.dll"
        IMPORTED_IMPLIB "${OBS_BUILD_DIR}/libobs/Release/obs.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OBS_BUILD_DIR}/../libobs"
    )

    add_library(OBS::obs-frontend-api SHARED IMPORTED)
    set_target_properties(OBS::obs-frontend-api PROPERTIES
        IMPORTED_LOCATION "${OBS_BUILD_DIR}/frontend/api/Release/obs-frontend-api.dll"
        IMPORTED_IMPLIB "${OBS_BUILD_DIR}/frontend/api/Release/obs-frontend-api.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OBS_BUILD_DIR}/../frontend/api"
    )

    # Add required include directories
    include_directories(
        "${OBS_BUILD_DIR}/../libobs"
        "${OBS_BUILD_DIR}/config"
        "${OBS_BUILD_DIR}/../frontend/api"
        "${OBS_BUILD_DIR}/../deps/w32-pthreads"
    )

    set(libobs_FOUND TRUE)
    set(obs-frontend-api_FOUND TRUE)
    message(STATUS "Using OBS from build directory")
else()
    # Try find_package
    find_package(libobs QUIET)
    find_package(obs-frontend-api QUIET)
endif()

# If not found, try to use installed OBS with source headers
if(NOT libobs_FOUND OR NOT obs-frontend-api_FOUND)
    message(STATUS "OBS packages not found via find_package, trying manual detection...")

    # OBS source directory for headers
    set(OBS_SOURCE_DIR "C:/obs-studio-src")

    # Common OBS installation paths for binaries
    set(OBS_INSTALL_PATHS
        "C:/Program Files/obs-studio"
        "C:/Program Files (x86)/obs-studio"
        "$ENV{ProgramFiles}/obs-studio"
        "$ENV{ProgramW6432}/obs-studio"
    )

    foreach(path ${OBS_INSTALL_PATHS})
        if(EXISTS "${path}")
            set(OBS_INSTALL_DIR "${path}")
            message(STATUS "Found OBS installation at: ${OBS_INSTALL_DIR}")
            break()
        endif()
    endforeach()

    if(NOT OBS_INSTALL_DIR)
        message(FATAL_ERROR "Could not find OBS Studio installation")
    endif()

    # Check if OBS source exists for headers
    if(NOT EXISTS "${OBS_SOURCE_DIR}/libobs")
        message(FATAL_ERROR "OBS source not found at ${OBS_SOURCE_DIR}. Please clone OBS source.")
    endif()

    # Generated .lib files location (created from DLLs using generate-libs.ps1)
    set(OBS_LIB_DIR "C:/obs-libs")

    # Create imported targets using installed binaries + source headers + generated libs
    add_library(OBS::libobs SHARED IMPORTED)
    set_target_properties(OBS::libobs PROPERTIES
        IMPORTED_LOCATION "${OBS_INSTALL_DIR}/bin/64bit/obs.dll"
        IMPORTED_IMPLIB "${OBS_LIB_DIR}/obs.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OBS_SOURCE_DIR}/libobs"
    )

    add_library(OBS::obs-frontend-api SHARED IMPORTED)
    set_target_properties(OBS::obs-frontend-api PROPERTIES
        IMPORTED_LOCATION "${OBS_INSTALL_DIR}/bin/64bit/obs-frontend-api.dll"
        IMPORTED_IMPLIB "${OBS_LIB_DIR}/obs-frontend-api.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OBS_SOURCE_DIR}/UI/obs-frontend-api"
    )

    # Add additional include directories needed for OBS headers
    include_directories(
        "${OBS_SOURCE_DIR}/libobs"
        "${OBS_SOURCE_DIR}/UI/obs-frontend-api"
    )

    message(STATUS "Using OBS binaries from ${OBS_INSTALL_DIR}, headers from ${OBS_SOURCE_DIR}")
endif()

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

# Note: cURL is not actually used - we use Qt Network instead
# find_package(CURL REQUIRED)

# Find nlohmann_json for JSON parsing
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # Set policy for older CMake compatibility in nlohmann_json
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
    FetchContent_MakeAvailable(json)
endif()

# Plugin sources
set(PLUGIN_SOURCES
    src/plugin-main.cpp
    src/youtube-chat-client.cpp
    src/obstruction-manager.cpp
    src/settings-dialog.cpp
    src/effect-system.cpp
    src/effect-config.cpp
    src/effect-config-dialog.cpp
    src/effect-config-manager.cpp
    src/room-3d-source.cpp
)

set(PLUGIN_HEADERS
    src/plugin-main.hpp
    src/youtube-chat-client.hpp
    src/obstruction-manager.hpp
    src/settings-dialog.hpp
    src/effect-system.hpp
    src/effect-config.hpp
    src/effect-config-dialog.hpp
    src/effect-config-manager.hpp
    src/room-3d-source.hpp
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
        nlohmann_json::nlohmann_json
    )
else()
    target_link_libraries(obs-youtube-superchat-plugin
        OBS::libobs
        OBS::obs-frontend-api
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
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

# Copy effect files to build directory for development
file(GLOB EFFECT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/effects/*.effect")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/data/obs-plugins/obs-youtube-superchat-plugin/effects")
foreach(effect_file ${EFFECT_FILES})
    get_filename_component(effect_name ${effect_file} NAME)
    configure_file(${effect_file}
        "${CMAKE_BINARY_DIR}/data/obs-plugins/obs-youtube-superchat-plugin/effects/${effect_name}"
        COPYONLY)
endforeach()

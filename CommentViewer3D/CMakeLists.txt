cmake_minimum_required(VERSION 3.20)
project(CommentViewer3D VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    OpenGL
    OpenGLWidgets
    Network
)

# Find Bullet Physics
find_package(Bullet REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/comment-viewer-window.cpp
    src/physics-engine.cpp
    src/comment-3d.cpp
    src/tcp-receiver.cpp
    src/gl-widget.cpp
)

# Header files
set(HEADERS
    include/comment-viewer-window.hpp
    include/physics-engine.hpp
    include/comment-3d.hpp
    include/tcp-receiver.hpp
    include/gl-widget.hpp
)

# Create executable
add_executable(CommentViewer3D ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(CommentViewer3D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${BULLET_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(CommentViewer3D PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Network
    ${BULLET_LIBRARIES}
)

# Windows specific settings
if(WIN32)
    set_target_properties(CommentViewer3D PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Installation
install(TARGETS CommentViewer3D
    RUNTIME DESTINATION bin
)

cmake_minimum_required(VERSION 3.16)
project(LightweightTextEditor VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options for low-end optimization
option(ENABLE_OPENGL "Use OpenGL rendering" ON)
option(BUILD_STATIC "Build static executable" OFF)
option(OPTIMIZE_SIZE "Optimize for binary size" OFF)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# Fetch external dependencies (pugixml for XML/DOCX, minizip for DOCX archives)
include(FetchContent)

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

FetchContent_Declare(
    minizip
    GIT_REPOSITORY https://github.com/nmoinvaz/minizip.git
    GIT_TAG master
)
FetchContent_MakeAvailable(minizip)

# Collect source files
set(SOURCES
    src/ui/MainWindow.cpp
    src/ui/Toolbar.cpp
    src/core/Document.cpp
    src/core/UndoManager.cpp
    src/editor/TextEditor.cpp
    src/editor/VirtualRenderer.cpp
    src/utils/FileHandler.cpp
    src/formats/DocxHandler.cpp
    src/formats/RtfHandler.cpp
    src/fonts/FontManager.cpp
)

set(HEADERS
    src/core/Document.h
    src/core/UndoManager.h
    src/editor/TextEditor.h
    src/editor/VirtualRenderer.h
    src/ui/MainWindow.h
    src/ui/Toolbar.h
    src/utils/FileHandler.h
    src/formats/DocxHandler.h
    src/formats/RtfHandler.h
    src/fonts/FontManager.h
)

# Create executable
add_executable(texteditor ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(texteditor PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    src/
    ${pugixml_SOURCE_DIR}/src
    ${minizip_SOURCE_DIR}
)

# Link libraries
target_link_libraries(texteditor PRIVATE
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    pugixml::pugixml
    minizip
    z
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(texteditor PRIVATE imm32 ole32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(texteditor PRIVATE pthread dl)
endif()

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(OPTIMIZE_SIZE)
        target_compile_options(texteditor PRIVATE -Os -flto)
        target_link_options(texteditor PRIVATE -flto)
    else()
        target_compile_options(texteditor PRIVATE -O3 -march=native)
    endif()
endif()

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS texteditor DESTINATION bin)
install(DIRECTORY assets DESTINATION share/texteditor)

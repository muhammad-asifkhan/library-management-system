cmake_minimum_required(VERSION 3.15)
project(LibraryManagementSystem VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
option(ENABLE_WERROR "Treat warnings as errors" OFF)
if(MSVC)
    add_compile_options(/W4)
    if(ENABLE_WERROR)
        add_compile_options(/WX)
    endif()
else()
    add_compile_options(-Wall -Wextra -pedantic)
    if(ENABLE_WERROR)
        add_compile_options(-Werror)
    endif()
endif()

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Optimization flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/core)
include_directories(${CMAKE_SOURCE_DIR}/include/data_structures)
include_directories(${CMAKE_SOURCE_DIR}/include/utils)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/core/*.cpp"
    "src/data_structures/*.cpp"
    "src/utils/*.cpp"
)

# Optional sanitizers (best for Debug/RelWithDebInfo)
option(ENABLE_SANITIZERS "Enable Address/Undefined sanitizers (non-MSVC)" OFF)
if(ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# GUI target (Dear ImGui + ImGui-SFML + SFML)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

include(FetchContent)

# Dear ImGui (real upstream)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.9
)

# ImGui-SFML (real upstream)
FetchContent_Declare(
    imgui_sfml
    GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml.git
    GIT_TAG v2.6
)

FetchContent_Populate(imgui)
FetchContent_Populate(imgui_sfml)

add_library(ImGui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(ImGui PUBLIC ${imgui_SOURCE_DIR})

add_library(ImGuiSFML STATIC
    ${imgui_sfml_SOURCE_DIR}/imgui-SFML.cpp
)
target_include_directories(ImGuiSFML PUBLIC ${imgui_sfml_SOURCE_DIR})
target_link_libraries(ImGuiSFML PUBLIC ImGui sfml-graphics sfml-window sfml-system OpenGL::GL)

add_executable(library_gui
    src/main_gui.cpp
    src/gui/MainWindow.cpp
    ${SOURCES}
)

target_link_libraries(library_gui
    ImGuiSFML
    sfml-graphics
    sfml-window
    sfml-system
    OpenGL::GL
    pthread
)

if(WIN32)
    target_link_libraries(library_gui opengl32)
endif()

# Create data directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)

# Install target
install(TARGETS library_gui DESTINATION bin)
install(DIRECTORY data/ DESTINATION data)

# Testing (optional)
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()

    # Tests are lightweight executables using <cassert>,
    # so we build each test file as its own CTest entry (no external deps).
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

    foreach(TEST_SRC ${TEST_SOURCES})
        file(RELATIVE_PATH TEST_REL ${CMAKE_SOURCE_DIR}/tests ${TEST_SRC})
        string(REPLACE "/" "_" TEST_NAME ${TEST_REL})
        string(REPLACE "\\" "_" TEST_NAME ${TEST_NAME})
        string(REPLACE ".cpp" "" TEST_NAME ${TEST_NAME})
        string(REPLACE "-" "_" TEST_NAME ${TEST_NAME})

        add_executable(${TEST_NAME} ${TEST_SRC} ${SOURCES})
        target_link_libraries(${TEST_NAME} pthread)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()

# Documentation with Doxygen (optional)
option(BUILD_DOCS "Build documentation" OFF)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)
        
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# Print configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build docs: ${BUILD_DOCS}")
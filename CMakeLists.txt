cmake_minimum_required(VERSION 3.14)
project(flecs-explorer C)

# Set C standard to C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)

# Set final binary output directory to build/debug or build/release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}")

# Strict compiler warning flags per docs/CODING_STANDARDS.md
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor/cJSON
    ${CMAKE_SOURCE_DIR}/vendor/flecs/include
)

# Subdirectories for vendored submodules
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_UTILS OFF CACHE BOOL "" FORCE)
set(FLECS_STATIC ON CACHE BOOL "" FORCE)
set(FLECS_SHARED OFF CACHE BOOL "" FORCE)

if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/cJSON/CMakeLists.txt")
    add_subdirectory(vendor/cJSON EXCLUDE_FROM_ALL)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/flecs/CMakeLists.txt")
    add_subdirectory(vendor/flecs EXCLUDE_FROM_ALL)
endif()

# System libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncursesw)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

# Main executable target
add_executable(flecs-explorer
    src/main.c
    src/terminal.c
    src/layout.c
)

target_include_directories(flecs-explorer PRIVATE
    ${NCURSES_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(flecs-explorer PRIVATE
    ${NCURSES_LIBRARIES}
    CURL::libcurl
    cjson
    flecs_static
    Threads::Threads
)

# Enable testing
enable_testing()
add_subdirectory(tests)

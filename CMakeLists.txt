cmake_minimum_required(VERSION 3.16)

project(flecs_explorer
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "Terminal User Interface (TUI) for Flecs ECS REST API"
)

# Standard C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Default to Debug build type if not explicitly set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Binary output directory routing to <project_root>/build/debug or <project_root>/build/release
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}/lib")

# Compiler warnings and build profile options
add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Build profile [Debug]: Enabling AddressSanitizer and UndefinedBehaviorSanitizer (-fsanitize=address,undefined)")
        add_compile_options(-g3 -O0 -fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Build profile [Release]: High optimization (-O3) with sanitizers stripped")
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Find required system dependencies
find_package(CURL REQUIRED)
find_package(Curses REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Vendored cJSON static library
add_library(cjson_static STATIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cjson/cJSON.c")
target_include_directories(cjson_static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cjson")

# Include directories for project headers
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cjson"
    ${CURSES_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Main application executable (conditional on src/main.c existence)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
    file(GLOB_RECURSE APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
    add_executable(flecs_explorer ${APP_SOURCES})
    target_link_libraries(flecs_explorer PRIVATE
        cjson_static
        ${CURSES_LIBRARIES}
        ${CURL_LIBRARIES}
        Threads::Threads
    )
endif()

# Test runner executable and CTest registration (conditional on tests/main.c existence)
enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.c")
    file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c")
    file(GLOB_RECURSE CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
    if(CORE_SOURCES)
        list(REMOVE_ITEM CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
    endif()

    add_executable(flecs_explorer_tests ${TEST_SOURCES} ${CORE_SOURCES})
    target_link_libraries(flecs_explorer_tests PRIVATE
        cjson_static
        ${CURSES_LIBRARIES}
        ${CURL_LIBRARIES}
        Threads::Threads
    )

    add_test(NAME AllTests COMMAND flecs_explorer_tests --all)
    add_test(NAME JsonTests COMMAND flecs_explorer_tests json)
endif()

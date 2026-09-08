cmake_minimum_required(VERSION 3.15)
project(CPP_Compat VERSION 1.0.0 LANGUAGES CXX)

# Support CMAKE_CXX_STANDARD (11, 14, 17, 20, 23). Default to 20 if not specified.
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to force fallback shims instead of std
option(COMPAT_FORCE_FALLBACK "Force use of self-contained fallback shims instead of std" OFF)

# HEADER-ONLY interface target
add_library(compat INTERFACE)
target_include_directories(compat INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(COMPAT_FORCE_FALLBACK)
    target_compile_definitions(compat INTERFACE COMPAT_FORCE_FALLBACK=1)
endif()

# Strict warning flags (MSVC /W4, GCC -Wall -Wextra)
if(MSVC)
    target_compile_options(compat INTERFACE
        /W4
        /utf-8
        /Zc:__cplusplus
    )
else()
    target_compile_options(compat INTERFACE
        -Wall
        -Wextra
    )
endif()

# Application target: compat_app (src/main.cpp)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(compat_app src/main.cpp)
    target_link_libraries(compat_app PRIVATE compat)
endif()

# Testing target: compat_test (tests/*.cpp)
enable_testing()

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
if(TEST_SOURCES)
    add_executable(compat_test ${TEST_SOURCES})
    target_link_libraries(compat_test PRIVATE compat)
    add_test(NAME compat_unit_tests COMMAND compat_test)
endif()

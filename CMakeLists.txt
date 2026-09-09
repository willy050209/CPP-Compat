cmake_minimum_required(VERSION 3.15)
project(CPP_Compat VERSION 1.0.0 LANGUAGES CXX)

# Support CMAKE_CXX_STANDARD (11, 14, 17, 20, 23). Default to 20 if not specified.
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options to force fallback / self-implementation shims instead of std
option(COMPAT_FORCE_FALLBACK "Force use of self-contained fallback shims instead of std" OFF)
option(COMPAT_FORCE_SELF_IMPLEMENTATION "Force use of self-contained fallback shims instead of std" OFF)

# HEADER-ONLY interface target
add_library(compat INTERFACE)
target_include_directories(compat INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(COMPAT_FORCE_FALLBACK OR COMPAT_FORCE_SELF_IMPLEMENTATION)
    target_compile_definitions(compat INTERFACE COMPAT_FORCE_FALLBACK=1 COMPAT_FORCE_SELF_IMPLEMENTATION=1)
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

# Benchmark target: compat_benchmark (benchmark/*.cpp)
option(COMPAT_BUILD_BENCHMARKS "Build benchmark suite" ON)

if(COMPAT_BUILD_BENCHMARKS)
    file(GLOB BENCHMARK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp")
    if(BENCHMARK_SOURCES)
        add_executable(compat_benchmark ${BENCHMARK_SOURCES})
        target_include_directories(compat_benchmark PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/benchmark")
        target_link_libraries(compat_benchmark PRIVATE compat)
        set_target_properties(compat_benchmark PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
        )
        if(MSVC)
            target_compile_options(compat_benchmark PRIVATE /std:c++latest /O2)
        else()
            target_compile_options(compat_benchmark PRIVATE -std=c++23 -O3)
        endif()
    endif()
endif()

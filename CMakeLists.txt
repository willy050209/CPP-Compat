cmake_minimum_required(VERSION 3.14...3.30)
project(compat VERSION 1.0.0 LANGUAGES CXX)

# Top-level project detection
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(COMPAT_IS_TOP_LEVEL ON)
else()
    set(COMPAT_IS_TOP_LEVEL OFF)
endif()

# Build options
option(COMPAT_BUILD_TESTS "Build unit tests" ${COMPAT_IS_TOP_LEVEL})
option(COMPAT_BUILD_BENCHMARKS "Build benchmark suite" OFF)
option(COMPAT_BUILD_EXAMPLES "Build example executable" ${COMPAT_IS_TOP_LEVEL})
option(COMPAT_BUILD_MODULE "Build C++20 module target from dist/compat.ixx" OFF)
option(COMPAT_INSTALL "Generate install and packaging targets" ${COMPAT_IS_TOP_LEVEL})
option(COMPAT_FORCE_SELF_IMPL "Force C++11 self-implementation across all standards for ABI stability" OFF)
option(COMPAT_FORCE_SELF_IMPLEMENTATION "Force C++11 self-implementation across all standards for ABI stability" OFF)
option(COMPAT_FORCE_FALLBACK "Force C++11 self-implementation across all standards for ABI stability" OFF)

# Standard selection: Defaults to C++11 for maximum backward compatibility baseline
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

# Header-only interface target
add_library(compat INTERFACE)
add_library(compat::compat ALIAS compat)

target_include_directories(compat INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(compat INTERFACE cxx_std_11)

# Compiler warnings and compatibility options
target_compile_options(compat INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus /utf-8 /W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# Multi-threading support (SharedMutex & Print)
find_package(Threads REQUIRED)
target_link_libraries(compat INTERFACE Threads::Threads)

# GCC / Clang < 9.0 filesystem linkage compatibility
target_link_libraries(compat INTERFACE
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:c++fs>
)

if(COMPAT_FORCE_SELF_IMPL OR COMPAT_FORCE_SELF_IMPLEMENTATION OR COMPAT_FORCE_FALLBACK)
    target_compile_definitions(compat INTERFACE COMPAT_FORCE_SELF_IMPLEMENTATION=1)
endif()

# Application target: compat_app (src/main.cpp)
if(COMPAT_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(compat_app src/main.cpp)
    target_link_libraries(compat_app PRIVATE compat)
endif()

# Unit tests target: compat_test (tests/*.cpp)
if(COMPAT_BUILD_TESTS)
    enable_testing()
    file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    if(TEST_SOURCES)
        add_executable(compat_test ${TEST_SOURCES})
        target_link_libraries(compat_test PRIVATE compat)
        add_test(NAME compat_unit_tests COMMAND compat_test)
    endif()
endif()

# Benchmark target: compat_benchmark (benchmark/*.cpp)
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

# Installation and packaging export
if(COMPAT_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS compat
        EXPORT compatTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT compatTargets
        FILE compatTargets.cmake
        NAMESPACE compat::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/compat
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compatConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/compatConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/compat
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/compatConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/compatConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/compatConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/compat
    )
endif()

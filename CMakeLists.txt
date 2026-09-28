# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.15)


project(g2basic 
    LANGUAGES C
    VERSION 0.1.1
    DESCRIPTION "G2Basic interpreter for Microcontrollers"
)

# Build examples by default only when configuring this project directly.
set(G2BASIC_BUILD_EXAMPLES_DEFAULT OFF)
set(G2BASIC_ENABLE_MATH_DEFAULT OFF)
set(G2BASIC_ENABLE_MATH_FUNCTIONS_DEFAULT OFF)
set(G2BASIC_MAX_NESTING_DEFAULT 8)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(G2BASIC_BUILD_EXAMPLES_DEFAULT ON)
    set(G2BASIC_ENABLE_MATH_DEFAULT ON)
    set(G2BASIC_ENABLE_MATH_FUNCTIONS_DEFAULT ON)
    set(G2BASIC_MAX_NESTING_DEFAULT 32)
endif()
option(G2BASIC_BUILD_EXAMPLES "Build G2Basic examples" ${G2BASIC_BUILD_EXAMPLES_DEFAULT})

option(G2BASIC_ENABLE_MATH_FUNCTIONS "Register built-in math functions" ${G2BASIC_ENABLE_MATH_FUNCTIONS_DEFAULT})

option(G2BASIC_ENABLE_MATH "Use libm for built-in math functions instead of approximations"
    ${G2BASIC_ENABLE_MATH_DEFAULT})

# Each nesting level costs roughly 120-200 bytes of C stack; see docs/embedding.md.
set(G2BASIC_MAX_NESTING ${G2BASIC_MAX_NESTING_DEFAULT} CACHE STRING
    "Maximum nesting of parentheses, unary signs, function calls, and IF-THEN per line")
if(NOT G2BASIC_MAX_NESTING MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "G2BASIC_MAX_NESTING must be a positive integer, got '${G2BASIC_MAX_NESTING}'")
endif()

add_library(${PROJECT_NAME})

add_subdirectory(src)
target_compile_definitions(${PROJECT_NAME} PRIVATE G2BASIC_MAX_NESTING=${G2BASIC_MAX_NESTING})

# Compiler options
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if(G2BASIC_ENABLE_MATH_FUNCTIONS AND G2BASIC_ENABLE_MATH)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

if(G2BASIC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# API documentation target, only for standalone builds.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${PROJECT_SOURCE_DIR}/cmake/docs.cmake)
endif()

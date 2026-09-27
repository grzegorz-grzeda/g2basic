# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.15)


project(g2basic 
    LANGUAGES C
    VERSION 0.1.0
    DESCRIPTION "G2Basic interpreter for Microcontrollers"
)

# Build examples by default only when configuring this project directly.
set(G2BASIC_BUILD_EXAMPLES_DEFAULT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(G2BASIC_BUILD_EXAMPLES_DEFAULT ON)
endif()
option(G2BASIC_BUILD_EXAMPLES "Build G2Basic examples" ${G2BASIC_BUILD_EXAMPLES_DEFAULT})

add_library(${PROJECT_NAME})

add_subdirectory(src)

# Compiler options
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Link math library
target_link_libraries(${PROJECT_NAME} m)

if(G2BASIC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

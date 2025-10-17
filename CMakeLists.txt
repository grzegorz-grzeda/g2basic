# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.15)


project(g2basic 
    LANGUAGES C
    VERSION 0.0.1
    DESCRIPTION "G2Basic interpreter for Microcontrollers"
)

add_library(${PROJECT_NAME})

add_subdirectory(src)

# Compiler options
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Link math library
target_link_libraries(${PROJECT_NAME} m)

add_subdirectory(examples)
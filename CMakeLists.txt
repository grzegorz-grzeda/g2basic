# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.15)


project(g2basic 
    LANGUAGES C
    VERSION 0.0.1
    DESCRIPTION "G2Basic interpreter for Microcontrollers"
)

add_executable(${PROJECT_NAME})

add_subdirectory(src)
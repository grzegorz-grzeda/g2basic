# SPDX-License-Identifier: MIT
#
project(g2basic-interactive)
add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} g2basic)
# SPDX-License-Identifier: MIT
#
target_sources(${PROJECT_NAME} PRIVATE
    expr.c
    main.c
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
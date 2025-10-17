# SPDX-License-Identifier: MIT
#
target_sources(${PROJECT_NAME}
    PRIVATE g2basic.c
)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
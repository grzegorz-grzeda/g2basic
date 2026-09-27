# SPDX-License-Identifier: MIT
#
target_sources(${PROJECT_NAME}
    PRIVATE g2basic.c
)

if(NOT G2BASIC_ENABLE_MATH_FUNCTIONS)
    target_sources(${PROJECT_NAME} PRIVATE g2basic_math_none.c)
elseif(G2BASIC_ENABLE_MATH)
    target_sources(${PROJECT_NAME} PRIVATE g2basic_math.c)
else()
    target_sources(${PROJECT_NAME} PRIVATE g2basic_math_crude.c)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
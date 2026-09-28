# SPDX-License-Identifier: MIT
#
# Optional API documentation target for standalone builds. The top-level
# CMakeLists.txt includes this file only when G2Basic is the main project, so
# embedding projects never get a conflicting `docs` target. The repository
# Doxyfile is included unchanged; this wrapper sets the version, the output
# location in the build directory, and the Graphviz path.

find_package(Doxygen OPTIONAL_COMPONENTS dot)

if(NOT TARGET Doxygen::doxygen OR NOT TARGET Doxygen::dot)
    message(STATUS "Doxygen or Graphviz dot not found; 'docs' target disabled")
    return()
endif()

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/external/doxygen-awesome-css/doxygen-awesome.css)
    message(STATUS "Documentation theme submodule missing; 'docs' target disabled. "
                   "Run: git submodule update --init external/doxygen-awesome-css")
    return()
endif()

get_filename_component(G2BASIC_DOT_DIR ${DOXYGEN_DOT_EXECUTABLE} DIRECTORY)
set(G2BASIC_DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.docs)

# The Doxyfile places HTML in the docs/ subdirectory of OUTPUT_DIRECTORY.
file(WRITE ${G2BASIC_DOXYFILE}
    "@INCLUDE         = \"${PROJECT_SOURCE_DIR}/Doxyfile\"\n"
    "PROJECT_NUMBER   = \"${PROJECT_VERSION}\"\n"
    "OUTPUT_DIRECTORY = \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
    "DOT_PATH         = \"${G2BASIC_DOT_DIR}\"\n"
)

# Paths in the Doxyfile are relative to the repository root.
add_custom_target(docs
    COMMAND Doxygen::doxygen ${G2BASIC_DOXYFILE}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating API documentation in ${CMAKE_CURRENT_BINARY_DIR}/docs"
    VERBATIM
)

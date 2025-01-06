set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/InverseKinematic.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/InverseKinematic.cpp"
)

set(LIBRARY_NAME "Shared")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")

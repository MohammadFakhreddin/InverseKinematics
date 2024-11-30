########################################

set(EXECUTABLE "Visualization")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/VisualizationMain.cpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################
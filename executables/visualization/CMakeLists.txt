########################################

set(EXECUTABLE "Visualization")

set(EXECUTABLE_RESOURCES
)

list(
    APPEND EXECUTABLE_RESOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/VisualizationMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VisualizationApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VisualizationApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderPass.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderPass.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderResource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderResource.hpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################
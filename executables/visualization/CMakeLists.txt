########################################

set(EXECUTABLE "Visualization")

list(
    APPEND EXECUTABLE_RESOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/VisualizationMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VisualizationApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VisualizationApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderPass.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderPass.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderResource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderResource.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneFrameBuffer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneFrameBuffer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapeGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapeGenerator.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapePipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapePipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapeRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapeRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridRenderer.hpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})

########################################
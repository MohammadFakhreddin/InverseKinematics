#pragma once

#include "BedrockPath.hpp"
#include "LogicalDevice.hpp"
#include "RenderTypes.hpp"
#include "SceneRenderPass.hpp"
#include "ShapePipeline.hpp"
#include "ShapeRenderer.hpp"
#include "Time.hpp"
#include "UI.hpp"
#include "camera/ArcballCamera.hpp"

#include <SDL_events.h>
// TODO: I could have just exported some mesh from GLTF and use the mesh renderer class instead. Why do I do this to myself everytime?
class VisualizationApp
{
public:

    explicit VisualizationApp();

    ~VisualizationApp();

    void Run();

private:

    void Update(float deltaTime);

    void Render(MFA::RT::CommandRecordState & recordState);

    void Resize();

    void OnSDL_Event(SDL_Event* event);

    void OnUI(float deltaTimeSec);

    void PrepareSceneRenderPass();

    void ApplyUI_Style();

    void DisplaySceneWindow();

    void DisplayParametersWindow();

    // Render parameters
    std::shared_ptr<MFA::Path> _path{};
    MFA::LogicalDevice * _device{};
    std::shared_ptr<MFA::UI> _ui{};
    std::unique_ptr<MFA::Time> _time{};
    std::shared_ptr<MFA::SwapChainRenderResource> _swapChainResource{};
    std::shared_ptr<MFA::DepthRenderResource> _depthResource{};
    std::shared_ptr<MFA::MSSAA_RenderResource> _msaaResource{};
    std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};
    std::shared_ptr<MFA::RT::SamplerGroup> _sampler{};

    std::shared_ptr<SceneFrameBuffer> _sceneFrameBuffer{};
    std::shared_ptr<SceneRenderResource> _sceneRenderResource{};
    std::shared_ptr<SceneRenderPass> _sceneRenderPass{};
    struct OldScene
    {
        std::shared_ptr<SceneRenderResource> sceneRenderResource{};
        std::shared_ptr<SceneFrameBuffer> sceneFrameBuffer{};
        std::vector<ImTextureID> textureIDs{};
        int remLifeTime{};
    };
    std::vector<OldScene> oldScenes{};
    std::vector<ImTextureID> _sceneTextureID_List{};
    VkExtent2D _sceneWindowSize{800, 800};
    bool _sceneWindowResized = false;
    bool _sceneWindowFocused = false;

    ImFont* _defaultFont{};
    ImFont* _boldFont{};

    std::shared_ptr<MFA::HostVisibleBufferTracker> _lightBufferTracker{};
    std::shared_ptr<MFA::HostVisibleBufferTracker> _cameraBufferTracker{};
    std::shared_ptr<MFA::ShapePipeline> _shapePipeline{};
    std::unique_ptr<ShapeRenderer> _cylinderShapeRenderer{};
    std::unique_ptr<ShapeRenderer> _sphereShapeRenderer{};

    std::unique_ptr<MFA::ArcballCamera> _camera{};

    int _activeImageIndex{};

    glm::vec3 _lightDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 _lightColor {1.0f, 1.0f, 1.0f};
    float _lightIntensity = 1.0f;
    float _specularLightIntensity = 1.0f;
    int _shininess = 32;
    float _ambientStrength = 0.25f;
};

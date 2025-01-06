#pragma once

#include "BedrockPath.hpp"
#include "LogicalDevice.hpp"
#include "RenderTypes.hpp"
#include "SceneRenderPass.hpp"
#include "Time.hpp"
#include "UI.hpp"

#include <SDL_events.h>

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

    // Render parameters
    std::shared_ptr<MFA::Path> _path{};
    std::unique_ptr<MFA::LogicalDevice> _device{};
    std::shared_ptr<MFA::UI> _ui{};
    std::unique_ptr<MFA::Time> _time{};
    std::shared_ptr<MFA::SwapChainRenderResource> _swapChainResource{};
    std::shared_ptr<MFA::DepthRenderResource> _depthResource{};
    std::shared_ptr<MFA::MSSAA_RenderResource> _msaaResource{};
    std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};
    std::shared_ptr<SceneRenderPass> _sceneRenderPass{};
    std::shared_ptr<SceneRenderResource> _sceneRenderResource{};

    ImFont* _defaultFont{};
    ImFont* _boldFont{};

};

#pragma once

#include "RenderTypes.hpp"
#include "BedrockPath.hpp"
#include "LogicalDevice.hpp"
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
    std::shared_ptr<MFA::Path> path{};
    std::unique_ptr<MFA::LogicalDevice> device{};
    std::shared_ptr<MFA::UI> ui{};
    std::unique_ptr<MFA::Time> time{};
    std::shared_ptr<MFA::SwapChainRenderResource> swapChainResource{};
    std::shared_ptr<MFA::DepthRenderResource> depthResource{};
    // TODO: We might have to disable this
    std::shared_ptr<MFA::MSSAA_RenderResource> msaaResource{};
    std::shared_ptr<MFA::DisplayRenderPass> displayRenderPass{};

    ImFont* defaultFont{};
    ImFont* boldFont{};

};

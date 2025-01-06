#pragma once

#include "BufferTracker.hpp"
#include "SceneRenderResource.hpp"
#include "render_resource/RenderResource.hpp"

class SceneRenderPass
{
public:

    explicit SceneRenderPass(std::shared_ptr<SceneRenderResource> renderResource);
    ~SceneRenderPass();

    // This is a special case so we don't need record state
    void Begin(VkCommandBuffer commandBuffer) const;

    void End(VkCommandBuffer commandBuffer);

    [[nodiscard]]
    VkRenderPass GetRenderPass() const;

private:

    void CreateRenderPass();

    void CreateFrameBuffer();

    std::shared_ptr<SceneRenderResource> _renderResource;
    std::unique_ptr<MFA::RT::RenderPass> _renderPass;
    std::unique_ptr<MFA::RT::FrameBuffer> _frameBuffer;

};

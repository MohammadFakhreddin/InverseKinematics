#pragma once

#include "BufferTracker.hpp"
#include "SceneRenderResource.hpp"

class SceneRenderPass
{
public:

    explicit SceneRenderPass(std::shared_ptr<SceneRenderResource> renderResource);
    ~SceneRenderPass();

    // This is a special case so we don't need record state
    void Begin(MFA::RT::CommandRecordState const & recordState) const;

    void End(MFA::RT::CommandRecordState const & recordState);

    [[nodiscard]]
    VkRenderPass GetRenderPass() const;

    void UpdateRenderResource(std::shared_ptr<SceneRenderResource> renderResource);

private:

    void CreateRenderPass();

    void CreateFrameBuffer();

    std::shared_ptr<SceneRenderResource> _renderResource;
    std::unique_ptr<MFA::RT::RenderPass> _renderPass;
    std::vector<std::unique_ptr<MFA::RT::FrameBuffer>> _frameBufferList;

};

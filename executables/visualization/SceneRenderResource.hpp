#pragma once

#include "RenderTypes.hpp"

class SceneRenderResource
{
public:

    explicit SceneRenderResource(VkExtent2D imageExtent, VkFormat imageFormat);
    ~SceneRenderResource();

    [[nodiscard]]
    VkExtent2D const & ImageExtent() const {return _imageExtent;}

    [[nodiscard]]
    VkFormat ImageFormat() const {return _imageFormat;}

    [[nodiscard]]
    VkSampleCountFlagBits MSAA_SampleCount() const {return _msaaSampleCount;}

    [[nodiscard]]
    MFA::RT::ColorImageGroup const & MSAA_Image() const {return *_msaaImage;}

    [[nodiscard]]
    std::shared_ptr<MFA::RT::ColorImageGroup> const & ColorImage() const {return _colorImage;}

    [[nodiscard]]
    std::shared_ptr<MFA::RT::DepthImageGroup> const & DepthImage() const {return _depthImage;}

private:

    VkExtent2D const _imageExtent;
    VkFormat const _imageFormat;
    VkSampleCountFlagBits _msaaSampleCount;

    std::shared_ptr<MFA::RT::ColorImageGroup> _msaaImage;
    std::shared_ptr<MFA::RT::ColorImageGroup> _colorImage;
    std::shared_ptr<MFA::RT::DepthImageGroup> _depthImage;

};

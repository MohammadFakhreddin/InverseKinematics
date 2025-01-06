#include "SceneRenderResource.hpp"

#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"
#include "render_resource/RenderResource.hpp"

using namespace MFA;

//======================================================================================================================

SceneRenderResource::SceneRenderResource(VkExtent2D imageExtent, VkFormat imageFormat) :
    _imageExtent(imageExtent), _imageFormat(imageFormat)
{
    auto *device = LogicalDevice::Instance;

    _msaaSampleCount = device->GetMaxSampleCount();

    {
        RB::CreateColorImageOptions params{};
        params.samplesCount = device->GetMaxSampleCount();
        params.imageType = VK_IMAGE_TYPE_2D;
        params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        _msaaImage = RB::CreateColorImage(
            device->GetPhysicalDevice(),
            device->GetVkDevice(),
            _imageExtent,
            _imageFormat,
            params
        );
    }

    {
        RB::CreateColorImageOptions params{};
        params.samplesCount = VK_SAMPLE_COUNT_1_BIT;
        params.imageType = VK_IMAGE_TYPE_2D;
        params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        _colorImage = RB::CreateColorImage(
            device->GetPhysicalDevice(),
            device->GetVkDevice(),
            _imageExtent,
            _imageFormat,
            params
        );
    }
}

//======================================================================================================================

SceneRenderResource::~SceneRenderResource() = default;

//======================================================================================================================
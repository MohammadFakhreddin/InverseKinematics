#include "SceneRenderResource.hpp"

#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

using namespace MFA;

//======================================================================================================================

SceneRenderResource::SceneRenderResource(VkExtent2D imageExtent, VkFormat imageFormat) :
    _imageExtent(imageExtent), _imageFormat(imageFormat)
{
    auto * device = LogicalDevice::Instance;

    _msaaSampleCount = VK_SAMPLE_COUNT_1_BIT;//device->GetMaxSampleCount();

    auto const maxImageCount = LogicalDevice::Instance->GetSwapChainImageCount();
    _msaaImageList.resize(maxImageCount);
    // _colorImageList.resize(maxImageCount);
    _depthImageList.resize(maxImageCount);
    for (int i = 0; i < maxImageCount; ++i)
    {
        {// MSAA Image
            RB::CreateColorImageOptions params{};
            params.samplesCount = VK_SAMPLE_COUNT_1_BIT;
            params.imageType = VK_IMAGE_TYPE_2D;
            params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            _msaaImageList[i] = RB::CreateColorImage(
                device->GetPhysicalDevice(),
                device->GetVkDevice(),
                _imageExtent,
                _imageFormat,
                params
            );
        }

        // {// Color Image
        //     RB::CreateColorImageOptions params{};
        //     params.samplesCount = VK_SAMPLE_COUNT_1_BIT;
        //     params.imageType = VK_IMAGE_TYPE_2D;
        //     params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        //         VK_IMAGE_USAGE_SAMPLED_BIT |
        //         VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        //
        //     _colorImageList[i] = RB::CreateColorImage(
        //         device->GetPhysicalDevice(),
        //         device->GetVkDevice(),
        //         _imageExtent,
        //         _imageFormat,
        //         params
        //     );
        // }

        {// Depth Image
            _depthImageList[i] = RB::CreateDepthImage(
                LogicalDevice::Instance->GetPhysicalDevice(),
                LogicalDevice::Instance->GetVkDevice(),
                _imageExtent,
                LogicalDevice::Instance->GetDepthFormat(),
                RB::CreateDepthImageOptions{
                    .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .samplesCount = VK_SAMPLE_COUNT_1_BIT
                }
            );
        }
    }
}

//======================================================================================================================

SceneRenderResource::~SceneRenderResource() = default;

//======================================================================================================================

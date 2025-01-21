#include "SceneRenderPass.hpp"

#include "LogicalDevice.hpp"

// Use Offscreen rendering:
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/offscreen/offscreen.cpp#L348

using namespace MFA;

//======================================================================================================================
// We need a renderResource per frame
SceneRenderPass::SceneRenderPass(std::shared_ptr<SceneRenderResource> renderResource)
    : _renderResource(std::move(renderResource))
{
    CreateRenderPass();
    CreateFrameBuffer();
}

//======================================================================================================================

SceneRenderPass::~SceneRenderPass() = default;

//======================================================================================================================

void SceneRenderPass::Begin(RT::CommandRecordState const & recordState) const
{
    auto const & imageExtent = _renderResource->ImageExtent();

    RB::AssignViewportAndScissorToCommandBuffer(imageExtent, recordState.commandBuffer);

    std::vector<VkClearValue> clearValues(2);
    clearValues[0].color = VkClearColorValue{ .float32 = {0.1f, 0.1f, 0.12f, 1.0f } };
    // clearValues[1].color = VkClearColorValue{ .float32 = {1.0f, 1.0f, 1.0f, 1.0f } };
    clearValues[1].depthStencil = { .depth = 1.0f, .stencil = 0 };

    RB::BeginRenderPass(
        recordState.commandBuffer,
        _renderPass->vkRenderPass,
        _frameBufferList[recordState.imageIndex]->framebuffer,
        imageExtent,
        static_cast<uint32_t>(clearValues.size()),
        clearValues.data()
    );
}

//======================================================================================================================

void SceneRenderPass::End(RT::CommandRecordState const & recordState)
{
    vkCmdEndRenderPass(recordState.commandBuffer);

    // VkImageSubresourceRange const subResourceRange{
    //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //     .baseMipLevel = 0,
    //     .levelCount = 1,
    //     .baseArrayLayer = 0,
    //     .layerCount = 1,
    // };
    //
    // VkImageMemoryBarrier const pipelineBarrier{
    //     .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    //     .srcAccessMask = 0,
    //     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    //     .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //     .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .image = _renderResource->ColorImage(recordState)->imageGroup->image,
    //     .subresourceRange = subResourceRange
    // };
    //
    // RB::PipelineBarrier(
    //     recordState.commandBuffer,
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     1,
    //     &pipelineBarrier
    // );
}

//======================================================================================================================

VkRenderPass SceneRenderPass::GetRenderPass() const
{
    return _renderPass->vkRenderPass;
}

//======================================================================================================================

void SceneRenderPass::CreateRenderPass()
{
    auto const * device = LogicalDevice::Instance;

    auto const surfaceFormat = _renderResource->ImageFormat();
    auto const depthFormat = LogicalDevice::Instance->GetDepthFormat();
    auto const sampleCount = _renderResource->MSAA_SampleCount();

    // Multi-sampled attachment that we render to
    VkAttachmentDescription const msaaAttachment{
        .format = surfaceFormat,
        .samples = sampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    // VkAttachmentDescription const resolveAttachment{
    //     .format = surfaceFormat,
    //     .samples = VK_SAMPLE_COUNT_1_BIT,
    //     .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    //     .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    //     .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    //     .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    //     .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    // };

    VkAttachmentDescription const depthAttachment{
        .format = depthFormat,
        .samples = sampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    // Note: hardware will automatically transition attachment to the specified layout
    // Note: index refers to attachment descriptions array
    constexpr VkAttachmentReference msaaAttachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    // constexpr VkAttachmentReference imageAttachmentReference{
    //     .attachment = 1,
    //     .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    // };

    constexpr VkAttachmentReference depthAttachmentReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    // Note: this is a description of how the attachments of the render pass will be used in this sub pass
    // e.g. if they will be read in shaders and/or drawn to
    std::vector<VkSubpassDescription> subPassDescription{
        VkSubpassDescription {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &msaaAttachmentReference,
            // .pResolveAttachments = &imageAttachmentReference,
            .pDepthStencilAttachment = &depthAttachmentReference,
        }
    };

    std::vector<VkAttachmentDescription> attachments = { msaaAttachment/*, resolveAttachment*/, depthAttachment };

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> subPassDependencies{};

    subPassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subPassDependencies[0].dstSubpass = 0;
    subPassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subPassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subPassDependencies[0].srcAccessMask = VK_ACCESS_NONE_KHR;
    subPassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subPassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subPassDependencies[1].srcSubpass = 0;
    subPassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subPassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subPassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subPassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subPassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subPassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    _renderPass = std::make_unique<RT::RenderPass>(RB::CreateRenderPass(
        device->GetVkDevice(),
        attachments.data(),
        static_cast<uint32_t>(attachments.size()),
        subPassDescription.data(),
        static_cast<uint32_t>(subPassDescription.size()),
        subPassDependencies.data(),
        subPassDependencies.size()
    ));
}

//======================================================================================================================

void SceneRenderPass::CreateFrameBuffer()
{
    auto const * device = LogicalDevice::Instance;

    auto const & extent = _renderResource->ImageExtent();

    auto const swapChainExtent = VkExtent2D{
        .width = extent.width,
        .height = extent.height
    };

    _frameBufferList.resize(device->GetSwapChainImageCount());

    for (int imageIndex = 0; imageIndex < _frameBufferList.size(); imageIndex++)
    {
        auto const & msaaImage = _renderResource->MSAA_Image(imageIndex);
        // auto const & colorImage = _renderResource->ColorImage(imageIndex);
        auto const & depthImage = _renderResource->DepthImage(imageIndex);

        std::vector<VkImageView> const attachments{
            msaaImage.imageView->imageView,
            // colorImage->imageView->imageView,
            depthImage->imageView->imageView
        };
        // We only need one framebuffer
        _frameBufferList[imageIndex] = std::make_unique<RT::FrameBuffer>(
            RB::CreateFrameBuffers(
                device->GetVkDevice(),
                _renderPass->vkRenderPass,
                attachments.data(),
                static_cast<uint32_t>(attachments.size()),
                swapChainExtent,
                1
            )
        );
    }
}

//======================================================================================================================

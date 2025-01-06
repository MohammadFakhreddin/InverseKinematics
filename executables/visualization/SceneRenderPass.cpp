#include "SceneRenderPass.hpp"

#include "LogicalDevice.hpp"

using namespace MFA;

//======================================================================================================================

SceneRenderPass::SceneRenderPass(std::shared_ptr<SceneRenderResource> renderResource)
    : _renderResource(std::move(renderResource))
{
    CreateRenderPass();
    CreateFrameBuffer();
}

//======================================================================================================================

SceneRenderPass::~SceneRenderPass() = default;

//======================================================================================================================

void SceneRenderPass::BeginRenderPass(VkCommandBuffer commandBuffer) const
{
    auto const & imageExtent = _renderResource->ImageExtent();

    RB::AssignViewportAndScissorToCommandBuffer(imageExtent, commandBuffer);

    std::vector<VkClearValue> clearValues(2);
    clearValues[0].color = VkClearColorValue{ .float32 = {1.0f, 1.0f, 1.0f, 0.0f } };
    clearValues[1].color = VkClearColorValue{ .float32 = {1.0f, 1.0f, 1.0f, 0.0f } };

    RB::BeginRenderPass(
        commandBuffer,
        _renderPass->vkRenderPass,
        _frameBuffer->framebuffer,
        imageExtent,
        static_cast<uint32_t>(clearValues.size()),
        clearValues.data()
    );
}

//======================================================================================================================

void SceneRenderPass::EndRenderPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
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
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription const resolveAttachment{
        .format = surfaceFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    // Note: hardware will automatically transition attachment to the specified layout
    // Note: index refers to attachment descriptions array
    constexpr VkAttachmentReference msaaAttachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    constexpr VkAttachmentReference imageAttachmentReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    // Note: this is a description of how the attachments of the render pass will be used in this sub pass
    // e.g. if they will be read in shaders and/or drawn to
    std::vector<VkSubpassDescription> subPassDescription{
        VkSubpassDescription {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &msaaAttachmentReference,
            .pResolveAttachments = &imageAttachmentReference
        }
    };

    std::vector<VkAttachmentDescription> attachments = { msaaAttachment, resolveAttachment };

    _renderPass = std::make_unique<RT::RenderPass>(RB::CreateRenderPass(
        device->GetVkDevice(),
        attachments.data(),
        static_cast<uint32_t>(attachments.size()),
        subPassDescription.data(),
        static_cast<uint32_t>(subPassDescription.size()),
        nullptr,
        0
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

    auto const & msaaImage = _renderResource->MSAA_Image();
    auto const & colorImage = _renderResource->ColorImage();

    std::vector<VkImageView> const attachments{
        msaaImage.imageView->imageView,
        colorImage->imageView->imageView
    };
    // We only need one framebuffer
    _frameBuffer = std::make_unique<RT::FrameBuffer>(RB::CreateFrameBuffers(
        device->GetVkDevice(),
        _renderPass->vkRenderPass,
        attachments.data(),
        static_cast<uint32_t>(attachments.size()),
        swapChainExtent,
        1
    ));
}

//======================================================================================================================

#include "SceneRenderPass.hpp"

#include "LogicalDevice.hpp"

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

void SceneRenderPass::Begin(VkCommandBuffer commandBuffer) const
{
    auto const & imageExtent = _renderResource->ImageExtent();

    RB::AssignViewportAndScissorToCommandBuffer(imageExtent, commandBuffer);

    std::vector<VkClearValue> clearValues(3);
    clearValues[0].color = VkClearColorValue{ .float32 = {0.1f, 0.1f, 0.12f, 1.0f } };
    clearValues[1].color = VkClearColorValue{ .float32 = {1.0f, 1.0f, 1.0f, 1.0f } };
    clearValues[2].depthStencil = { .depth = 1.0f, .stencil = 0 };

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

void SceneRenderPass::End(VkCommandBuffer commandBuffer)
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

    constexpr VkAttachmentReference imageAttachmentReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    constexpr VkAttachmentReference depthAttachmentReference{
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    // Note: this is a description of how the attachments of the render pass will be used in this sub pass
    // e.g. if they will be read in shaders and/or drawn to
    std::vector<VkSubpassDescription> subPassDescription{
        VkSubpassDescription {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &msaaAttachmentReference,
            .pResolveAttachments = &imageAttachmentReference,
            .pDepthStencilAttachment = &depthAttachmentReference,
        }
    };

    std::vector<VkAttachmentDescription> attachments = { msaaAttachment, resolveAttachment, depthAttachment };

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

    // TODO: For loop over the whole thing
    auto const & extent = _renderResource->ImageExtent();

    auto const swapChainExtent = VkExtent2D{
        .width = extent.width,
        .height = extent.height
    };

    auto const & msaaImage = _renderResource->MSAA_Image();
    auto const & colorImage = _renderResource->ColorImage();
    auto const & depthImage = _renderResource->DepthImage();

    std::vector<VkImageView> const attachments{
        msaaImage.imageView->imageView,
        colorImage->imageView->imageView,
        depthImage->imageView->imageView
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

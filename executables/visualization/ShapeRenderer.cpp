#include "ShapeRenderer.hpp"

#include <ranges>
#include <utility>

#include "LogicalDevice.hpp"

using namespace MFA;

//----------------------------------------------------------------------------------------------------------------------

ShapeRenderer::ShapeRenderer(
    std::shared_ptr<Pipeline> pipeline,

    std::shared_ptr<RT::BufferGroup> viewProjectionBuffer,
    std::shared_ptr<RT::BufferGroup> lightSourceBuffer,
    // Vertex count
    int const vertexCount,
    Vertex const *vertices,
    Normal const *normals,
    // Index count
    int const indexCount,
    Index const *indices,

    int const maxInstanceCount
) :
    mPipeline(std::move(pipeline)),
    mViewProjectionBuffer(std::move(viewProjectionBuffer)),
    mLightSourceBuffer(std::move(lightSourceBuffer)),
    mVertexCount(vertexCount),
    mIndexCount(indexCount),
    mMaxInstanceCount(maxInstanceCount)
{
    const auto device = LogicalDevice::Instance;

    const auto commandBuffer = RB::BeginSingleTimeCommand(device->GetVkDevice(), device->GetGraphicCommandPool());

    auto vertexStageBuffer = GenerateVertexBuffer(commandBuffer, mVertexCount, vertices, normals);
    auto indexStageBuffer = GenerateIndexBuffer(commandBuffer, mIndexCount, indices);

    RB::EndAndSubmitSingleTimeCommand(
        device->GetVkDevice(),
        device->GetGraphicCommandPool(),
        device->GetGraphicQueue(),
        commandBuffer
    );

    mDescriptorSetGroup = mPipeline->CreatePerRenderDescriptorSets(*mViewProjectionBuffer, *mLightSourceBuffer);

    {
        auto device = LogicalDevice::Instance;
        mInstanceBuffer = std::make_shared<HostVisibleBufferTracker>(RB::CreateBufferGroup(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            sizeof(Pipeline::Instance) * maxInstanceCount,
            device->GetMaxFramePerFlight(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        ));
    }
}

//----------------------------------------------------------------------------------------------------------------------

void ShapeRenderer::Queue(Pipeline::Instance const & instance)
{
    auto * data = mInstanceBuffer->Data();
    data += mInstanceCount * sizeof(Pipeline::Instance);
    memcpy(data, &instance, sizeof(Pipeline::Instance));
    mInstanceCount += 1;
}

//----------------------------------------------------------------------------------------------------------------------

void ShapeRenderer::Render(RT::CommandRecordState &recordState)
{
    mInstanceBuffer->Update(recordState);

    mPipeline->BindPipeline(recordState);

    RB::AutoBindDescriptorSet(recordState, RB::UpdateFrequency::PerPipeline, mDescriptorSetGroup);

    RB::BindIndexBuffer(
        recordState,
        *_indexBuffer,
        0,
        VK_INDEX_TYPE_UINT32
    );

    RB::BindVertexBuffer(
        recordState,
        *_vertexBuffer,
        0,
        0
    );

    RB::BindVertexBuffer(
        recordState,
        *mInstanceBuffer->HostVisibleBuffer()->buffers[recordState.frameIndex],
        1,
        0
    );

    RB::DrawIndexed(
        recordState,
        mIndexCount,
        mInstanceCount,
        0
    );

    mInstanceCount = 0;
}

//----------------------------------------------------------------------------------------------------------------------

std::shared_ptr<RT::BufferGroup> ShapeRenderer::GenerateVertexBuffer(
    VkCommandBuffer cb,
    int const vertexCount,
    Vertex const * vertices,
    Normal const * normals
)
{
    std::vector<Pipeline::Vertex> cpuVertices(vertexCount);

    for (int i = 0; i < mVertexCount; ++i)
    {
        cpuVertices[i] = Pipeline::Vertex
        {
            .position = vertices[i],
            .normal = normals[i]
        };
    }

    auto const alias = Alias(cpuVertices.data(), cpuVertices.size());

    auto const * device = LogicalDevice::Instance;

    auto const stageBuffer = RB::CreateStageBuffer(
        device->GetVkDevice(),
        device->GetPhysicalDevice(),
        alias.Len(),
        1
    );

    _vertexBuffer = RB::CreateVertexBuffer(
        device->GetVkDevice(),
        device->GetPhysicalDevice(),
        cb,
        *stageBuffer->buffers[0],
        alias
    );

    return stageBuffer;
}

//----------------------------------------------------------------------------------------------------------------------

std::shared_ptr<RT::BufferGroup> ShapeRenderer::GenerateIndexBuffer(
    VkCommandBuffer cb,
    int const indexCount,
    Index const * indices
)
{
    auto const * device = LogicalDevice::Instance;

    Alias const indexBlob {indices, sizeof(Index) * indexCount};

    auto const indexStageBuffer = RB::CreateStageBuffer(
        device->GetVkDevice(),
        device->GetPhysicalDevice(),
        indexBlob.Len(),
        1
    );

    _indexBuffer = RB::CreateIndexBuffer(
        device->GetVkDevice(),
        device->GetPhysicalDevice(),
        cb,
        *indexStageBuffer->buffers[0],
        indexBlob
    );

    return indexStageBuffer;
}

//----------------------------------------------------------------------------------------------------------------------



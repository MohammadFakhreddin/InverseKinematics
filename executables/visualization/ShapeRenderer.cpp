#include "ShapeRenderer.hpp"

#include <ranges>

#include "LogicalDevice.hpp"

using namespace MFA;

//----------------------------------------------------------------------------------------------------------------------

ShapeRenderer::ShapeRenderer(
    std::shared_ptr<Pipeline> pipeline,
    int const vertexCount,
    Vertex const * vertices,
    Normal const * normals,
    int const indexCount,
    Index const * indices
)
    : _pipeline(std::move(pipeline))
    , _vertexCount(vertexCount)
    , _indexCount(indexCount)
{
    const auto device = LogicalDevice::Instance;

    const auto commandBuffer = RB::BeginSingleTimeCommand(
        device->GetVkDevice(),
        device->GetGraphicCommandPool()
    );

    auto vertexStageBuffer = GenerateVertexBuffer(commandBuffer, _vertexCount, vertices, normals);
    auto indexStageBuffer = GenerateIndexBuffer(commandBuffer, _indexCount, indices);

    RB::EndAndSubmitSingleTimeCommand(
        device->GetVkDevice(),
        device->GetGraphicCommandPool(),
        device->GetGraphicQueue(),
        commandBuffer
    );
}

//----------------------------------------------------------------------------------------------------------------------

void ShapeRenderer::Render(
    RT::CommandRecordState &recordState,
    int const instanceCount,
    Pipeline::PushConstants const *instances
) const
{
    _pipeline->BindPipeline(recordState);

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

    for (int i = 0; i < instanceCount; ++i)
    {
        _pipeline->SetPushConstants(
            recordState,
            instances[i]
        );

        RB::DrawIndexed(
            recordState,
            _indexCount,
            1,
            0
        );
    }
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

    for (int i = 0; i < _vertexCount; ++i)
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

std::shared_ptr<MFA::RT::BufferGroup> ShapeRenderer::GenerateIndexBuffer(
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



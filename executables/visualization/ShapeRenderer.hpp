#pragma once

#include "AssetGLTF_Mesh.hpp"
#include "BufferTracker.hpp"
#include "ShapePipeline.hpp"

class ShapeRenderer
{
public:

    using Pipeline = ShapePipeline;
    using Vertex = glm::vec3;
    using Normal = glm::vec3;
    using Index = uint32_t;

    explicit ShapeRenderer(
        std::shared_ptr<Pipeline> pipeline,

        std::shared_ptr<MFA::RT::BufferGroup> viewProjectionBuffer,
        std::shared_ptr<MFA::RT::BufferGroup> lightSourceBuffer,

        int vertexCount,
        Vertex const * vertices,
        Normal const * normals,

        int indexCount,
        Index const * indices,

        int maxInstanceCount = 100
    );

    void Queue(Pipeline::Instance const & instance);

    void Render(MFA::RT::CommandRecordState & recordState);

private:

    std::shared_ptr<MFA::RT::BufferGroup> GenerateVertexBuffer(
        VkCommandBuffer cb,
        int vertexCount,
        Vertex const * vertices,
        Normal const * normals
    );

    std::shared_ptr<MFA::RT::BufferGroup> GenerateIndexBuffer(
        VkCommandBuffer cb,
        int indexCount,
        Index const * indices
    );

    std::shared_ptr<Pipeline> mPipeline{};

    MFA::RenderTypes::DescriptorSetGroup mDescriptorSetGroup{};

    std::shared_ptr<MFA::RT::BufferGroup> mViewProjectionBuffer{};
    std::shared_ptr<MFA::RT::BufferGroup> mLightSourceBuffer{};

    int mVertexCount{};
    int mIndexCount{};
    std::shared_ptr<MFA::RT::BufferAndMemory> _vertexBuffer{};
    std::shared_ptr<MFA::RT::BufferAndMemory> _indexBuffer{};

    int mMaxInstanceCount;
    std::shared_ptr<MFA::HostVisibleBufferTracker> mInstanceBuffer{};
    std::vector<Pipeline::Instance> mInstances{};

    int mInstanceCount{};
};

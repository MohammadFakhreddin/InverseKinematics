#pragma once

#include "AssetGLTF_Mesh.hpp"
#include "ShapePipeline.hpp"

class ShapeRenderer
{
public:

    using Pipeline = MFA::ShapePipeline;
    using Vertex = glm::vec3;
    using Normal = glm::vec3;
    using Index = int;

    explicit ShapeRenderer(
        std::shared_ptr<Pipeline> pipeline,
        int vertexCount,
        Vertex const * vertices,
        Normal const * normals,
        int indexCount,
        Index const * indices
    );

    void Render(
        MFA::RT::CommandRecordState &recordState,
        int instanceCount,
        Pipeline::PushConstants const *instances
    ) const;

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

    std::shared_ptr<Pipeline> _pipeline{};

    std::shared_ptr<MFA::RT::GpuTexture> _errorTexture{};

    int _vertexCount{};
    int _indexCount{};
    std::shared_ptr<MFA::RT::BufferAndMemory> _vertexBuffer{};
    std::shared_ptr<MFA::RT::BufferAndMemory> _indexBuffer{};
};

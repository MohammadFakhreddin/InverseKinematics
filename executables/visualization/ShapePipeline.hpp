#pragma once

#include "render_pass/DisplayRenderPass.hpp"
#include "pipeline/IShadingPipeline.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace MFA
{
    class ShapePipeline : public IShadingPipeline
    {
    public:

        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 normal{};
        };

        struct ViewProjection
        {
            glm::mat4 matrix{};
        };
        // Directional light for now
        struct LightSource
        {
            glm::vec3 direction{};
            float ambientStrength{};
            glm::vec3 color{};
            float placeholder0{};
        };

        struct PushConstants
        {
            glm::mat4 model;
            glm::vec4 color;
        };

        struct Material
        {
            glm::vec4 color {};
            int hasBaseColorTexture {};
            int specularStrength {};
            int shininess {};
            int placeholder2 {};
        };

        struct Params
        {
            int maxSets = 1000;
            VkCullModeFlags cullModeFlags = VK_CULL_MODE_BACK_BIT;
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        };

        explicit ShapePipeline(
            VkRenderPass renderPass,
            std::shared_ptr<RT::BufferGroup> viewProjectionBuffer,
            std::shared_ptr<RT::BufferGroup> lightSourceBuffer,
            Params params = Params {
                .maxSets = 100,
                .cullModeFlags = VK_CULL_MODE_BACK_BIT,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .polygonMode = VK_POLYGON_MODE_FILL
            }
        );

        ~ShapePipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const& recordState) const;

        void BindPipeline(RT::CommandRecordState& recordState) const;

        void SetPushConstants(RT::CommandRecordState& recordState, PushConstants pushConstants) const;

        void reload() override;

    private:

        void CreatePerPipelineDescriptorSetLayout();

        void CreatePipeline();

        void CreatePerPipelineDescriptorSets();

        std::shared_ptr<RT::DescriptorPool> mDescriptorPool{};

    	std::shared_ptr<RT::DescriptorSetLayoutGroup> mPerPipelineDescriptorLayout{};

        std::shared_ptr<RT::PipelineGroup> mPipeline{};
        std::shared_ptr<RT::BufferGroup> mViewProjBuffer{};
        std::shared_ptr<RT::BufferGroup> mLightSourceBuffer{};

        RT::DescriptorSetGroup mPerPipelineDescriptorSetGroup{};

        VkRenderPass mRenderPass{};

        Params _params{};
    };
}

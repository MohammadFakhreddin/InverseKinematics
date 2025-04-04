#pragma once

#include "render_pass/DisplayRenderPass.hpp"
#include "pipeline/IShadingPipeline.hpp"

#include <glm/glm.hpp>
#include <memory>
// TODO: Use spirv-reflect to make your life easier
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

        struct Instance
        {
            glm::mat4 model = glm::mat4(1.0f);
            glm::vec4 color = glm::vec4(1.0f);
            float specularStrength = 1.0f;
            int shininess = 32;
        };

        struct Camera
        {
            glm::mat4 viewProjection{};

            glm::vec3 position;
            float placeholder;
        };

        // Directional light for now
        struct LightSource
        {
            glm::vec3 direction{};
            float ambientStrength{};
            glm::vec3 color{};
            float placeholder0{};
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

        void Reload() override;

        [[nodiscard]]
        RT::DescriptorSetGroup CreatePerRenderDescriptorSets(
            const RT::BufferGroup & viewProjectionBuffer,
            const RT::BufferGroup & lightSourceBuffer
        ) const;

    private:

        void CreatePerRenderDescriptorSetLayout();

        void CreatePipeline();

        std::shared_ptr<RT::DescriptorPool> mDescriptorPool{};

    	std::shared_ptr<RT::DescriptorSetLayoutGroup> mPerPipelineDescriptorLayout{};

        std::shared_ptr<RT::PipelineGroup> mPipeline{};

        VkRenderPass mRenderPass{};

        Params _params{};
    };
}

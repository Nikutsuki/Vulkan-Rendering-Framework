#pragma once

#include "pch.h"

#include "pipelines/pipeline.h"
#include "device.h"
#include "game_object.h"
#include "object_manager_system.h"
#include "camera.h"
#include "pipelines/main_pipeline.h"
#include "skeletal_animations/gltf_model.h"

namespace game_engine {
    struct PushConstantData {
        glm::mat4 model_matrix{1.f};
        glm::vec3 color{0.f, 0.f, 0.f};
        int texture_index = 0;
    };

    class RenderSystem {
    public:
        RenderSystem(

            Device &device,
            VkRenderPass render_pass,
            VkDescriptorSetLayout global_set_layout,
            VkDescriptorSetLayout joint_set_layout,
            VkDescriptorSetLayout materials_set_layout
        );

        ~RenderSystem();
        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void render_game_objects(
            VkCommandBuffer command_buffer,
            game_engine::ObjectManagerSystem::ObjectMap &game_objects,
            const Camera &camera,
            const VkDescriptorSet global_descriptor_set,
            const VkDescriptorSet joint_descriptor_set,
            const VkDescriptorSet texture_descriptor_set
        );

        void render_wireframe_game_objects(
            VkCommandBuffer command_buffer,
            game_engine::ObjectManagerSystem::ObjectMap &game_objects,
            const Camera &camera,
            const VkDescriptorSet global_descriptor_set,
            const VkDescriptorSet joint_descriptor_set
        );
    private:
        void create_main_pipeline_layout(
            VkDescriptorSetLayout global_set_layout,
            VkDescriptorSetLayout joint_set_layout,
            VkDescriptorSetLayout texture_set_layout
        );

        void create_main_pipeline(VkRenderPass render_pass);

        void create_wireframe_pipeline_layout(
            VkDescriptorSetLayout global_set_layout,
            VkDescriptorSetLayout joint_set_layout,
            VkDescriptorSetLayout texture_set_layout
            );

        void create_wireframe_pipeline(VkRenderPass render_pass);

        Device &device;
        std::unique_ptr<MainPipeline> pipeline;
        VkPipelineLayout pipeline_layout;
        std::unique_ptr<Pipeline> wireframe_pipeline;
        VkPipelineLayout wireframe_pipeline_layout;
    };
}

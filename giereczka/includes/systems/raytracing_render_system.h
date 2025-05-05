//
// Created by Niku on 3/1/2025.
//
#pragma once

#include "pch.h"

#include "pipelines/raytracing_pipeline.h"
#include "camera.h"
#include "object_manager_system.h"
#include "render_system.h"
#include "descriptors.h"

namespace game_engine {
    class RaytracingRenderSystem {
    public:
        RaytracingRenderSystem(
            Device &device,
            VkRenderPass render_pass,
            VkDescriptorSetLayout global_set_layout,
            VkDescriptorSetLayout joint_set_layout,
            VkDescriptorSetLayout materials_set_layout,
            VkDescriptorSetLayout mesh_data_set_layout
        );

        ~RaytracingRenderSystem();

        RaytracingRenderSystem(const RaytracingRenderSystem &) = delete;

        RaytracingRenderSystem &operator=(const RaytracingRenderSystem &) = delete;

        void render_raytraced_scene(
            VkCommandBuffer command_buffer,
            game_engine::ObjectManagerSystem::ObjectMap &game_objects,
            const Camera &camera,
            const VkDescriptorSet global_descriptor_set,
            const VkDescriptorSet joint_descriptor_set,
            const VkDescriptorSet texture_descriptor_set,
            const VkDescriptorSet mesh_output_set,
            uint32_t width,
            uint32_t height
        );

        void create_mesh_storage_buffers(
            game_engine::ObjectManagerSystem::ObjectMap &game_objects
        );

        std::unique_ptr<Buffer> vertex_storage_buffer;
        std::unique_ptr<Buffer> index_storage_buffer;

    private:
        void create_raytracing_pipeline_layout(
            VkDescriptorSetLayout global_set_layout,
            VkDescriptorSetLayout joint_set_layout,
            VkDescriptorSetLayout texture_set_layout,
            VkDescriptorSetLayout mesh_data_set_layout
        );

        void create_raytracing_pipeline(VkRenderPass render_pass);

        Device &device;
        std::unique_ptr<RaytracingPipeline> raytracing_pipeline;
        VkPipelineLayout raytracing_pipeline_layout;
    };
}

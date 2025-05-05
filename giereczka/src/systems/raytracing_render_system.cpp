//
// Created by Niku on 3/1/2025.
//
#include "systems/raytracing_render_system.h"

game_engine::RaytracingRenderSystem::RaytracingRenderSystem(
    Device &device,
    VkRenderPass render_pass,
    VkDescriptorSetLayout global_set_layout,
    VkDescriptorSetLayout joint_set_layout,
    VkDescriptorSetLayout materials_set_layout,
    VkDescriptorSetLayout mesh_data_set_layout
) : device(device)
{
    create_raytracing_pipeline_layout(global_set_layout, joint_set_layout, materials_set_layout, mesh_data_set_layout);
    create_raytracing_pipeline(render_pass);
}

game_engine::RaytracingRenderSystem::~RaytracingRenderSystem()
{
    vkDestroyPipelineLayout(device.get_logical_device(), raytracing_pipeline_layout, nullptr);
}

void game_engine::RaytracingRenderSystem::render_raytraced_scene(
    VkCommandBuffer command_buffer,
    game_engine::ObjectManagerSystem::ObjectMap &
    game_objects, const Camera &camera,
    const VkDescriptorSet global_descriptor_set,
    const VkDescriptorSet joint_descriptor_set,
    const VkDescriptorSet texture_descriptor_set,
    const VkDescriptorSet mesh_output_set,
    uint32_t width,
    uint32_t height
)
{
    VkImage output_image{};

    // Transition the output image to general layout
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = nullptr;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier(
        command_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // Bind compute pipeline
    raytracing_pipeline->bind(command_buffer);

    std::vector<VkDescriptorSet> descriptor_sets{
        global_descriptor_set,
        joint_descriptor_set,
        texture_descriptor_set,
        mesh_output_set
    };

    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        raytracing_pipeline_layout,
        0,
        descriptor_sets.size(),
        descriptor_sets.data(),
        0,
        nullptr
    );

    // Push constants if needed
    PushConstantData push{};
    push.model_matrix = glm::mat4(1.0f);

    vkCmdPushConstants(
        command_buffer,
        raytracing_pipeline_layout,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(PushConstantData),
        &push
    );

    // Dispatch compute shader (8x8 workgroup size is used in shader)
    vkCmdDispatch(
        command_buffer,
        (width + 7) / 8,
        (height + 7) / 8,
        1
    );
}

void game_engine::RaytracingRenderSystem::create_raytracing_pipeline_layout(
    VkDescriptorSetLayout global_set_layout,
    VkDescriptorSetLayout joint_set_layout,
    VkDescriptorSetLayout texture_set_layout,
    VkDescriptorSetLayout mesh_data_set_layout
)
{
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PushConstantData);

    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
        global_set_layout, joint_set_layout, texture_set_layout, mesh_data_set_layout
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    if (vkCreatePipelineLayout(
            device.get_logical_device(),
            &pipeline_layout_info,
            nullptr,
            &raytracing_pipeline_layout
        ) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create raytracing pipeline layout");
    }
}

void game_engine::RaytracingRenderSystem::create_raytracing_pipeline(VkRenderPass render_pass)
{
    assert(raytracing_pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
    pipeline_config_info pipeline_config{};
    Pipeline::default_pipeline_config_info(pipeline_config);

    pipeline_config.render_pass = render_pass;
    pipeline_config.pipeline_layout = raytracing_pipeline_layout;
    raytracing_pipeline = std::make_unique<RaytracingPipeline>(
        device,
        "compiled_shaders/raytracing_compute_shader.comp.spv",
        pipeline_config
    );
}

void game_engine::RaytracingRenderSystem::create_mesh_storage_buffers(
    game_engine::ObjectManagerSystem::ObjectMap &game_objects)
{
    std::vector<Model::Vertex> all_vertices;
    std::vector<uint32_t> all_indices;

    // Collect all vertex and index data from game objects
    for (auto& obj : game_objects) {
        if (obj.second.gltf_model == nullptr) continue;

        for (auto& model : obj.second.gltf_model->models) {
            if (model == nullptr) continue;

            // You'll need to add methods to get vertices and indices from your Model class
            std::vector<Model::Vertex> vertices = model->get_vertices();
            std::vector<uint32_t> indices = model->get_indices();

            // Store base vertex for this model
            uint32_t base_vertex = all_vertices.size();

            // Add vertices
            all_vertices.insert(all_vertices.end(), vertices.begin(), vertices.end());

            for (uint32_t index : indices) {
                 all_indices.push_back(index + base_vertex);
             }
        }
    }

    // Create vertex buffer
    vertex_storage_buffer = std::make_unique<Buffer>(
        device,
        sizeof(Model::Vertex),
        all_vertices.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    vertex_storage_buffer->map();
    vertex_storage_buffer->write_to_buffer(all_vertices.data());

    // Create index buffer
    index_storage_buffer = std::make_unique<Buffer>(
        device,
        sizeof(uint32_t),
        all_indices.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    index_storage_buffer->map();
    index_storage_buffer->write_to_buffer(all_indices.data());
}

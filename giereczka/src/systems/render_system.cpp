#include "render_system.h"

#include "pipelines/wireframe_pipeline.h"

game_engine::RenderSystem::RenderSystem(
	Device& device,
	VkRenderPass render_pass,
	VkDescriptorSetLayout global_set_layout,
	VkDescriptorSetLayout joint_set_layout,
	VkDescriptorSetLayout materials_set_layout
) : device(device)
{
	create_main_pipeline_layout(global_set_layout, joint_set_layout, materials_set_layout);
	create_main_pipeline(render_pass);

	create_wireframe_pipeline_layout(global_set_layout, joint_set_layout, materials_set_layout);
	create_wireframe_pipeline(render_pass);
}

game_engine::RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(device.get_logical_device(), pipeline_layout, nullptr);
	vkDestroyPipelineLayout(device.get_logical_device(), wireframe_pipeline_layout, nullptr);
}

void game_engine::RenderSystem::create_main_pipeline_layout(
	VkDescriptorSetLayout global_set_layout,
	VkDescriptorSetLayout joint_set_layout,
	VkDescriptorSetLayout materials_set_layout
)
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstantData);

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts{ global_set_layout, joint_set_layout, materials_set_layout };

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
		&pipeline_layout
	) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

void game_engine::RenderSystem::create_main_pipeline(VkRenderPass render_pass)
{
	assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
	pipeline_config_info pipeline_config{};
	Pipeline::default_pipeline_config_info(pipeline_config);

	pipeline_config.render_pass = render_pass;
	pipeline_config.pipeline_layout = pipeline_layout;
	pipeline = std::make_unique<MainPipeline>(
		device,
		"compiled_shaders/vert_shader.vert.spv",
		"compiled_shaders/frag_shader.frag.spv",
		pipeline_config
	);
}

void game_engine::RenderSystem::create_wireframe_pipeline_layout(
	VkDescriptorSetLayout global_set_layout,
	VkDescriptorSetLayout joint_set_layout,
	VkDescriptorSetLayout materials_set_layout
)
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstantData);

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts{ global_set_layout, joint_set_layout, materials_set_layout };

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
		&wireframe_pipeline_layout
	) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

void game_engine::RenderSystem::create_wireframe_pipeline(VkRenderPass render_pass)
{
	assert(wireframe_pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
	pipeline_config_info pipeline_config{};
	Pipeline::default_pipeline_config_info(pipeline_config);

	pipeline_config.render_pass = render_pass;
	pipeline_config.pipeline_layout = wireframe_pipeline_layout;
	wireframe_pipeline = std::make_unique<WireframePipeline>(
		device,
		"compiled_shaders/wireframe_vert_shader.vert.spv",
		"compiled_shaders/wireframe_geom_shader.geom.spv",
		"compiled_shaders/wireframe_frag_shader.frag.spv",
		pipeline_config
	);
}

void game_engine::RenderSystem::render_game_objects(
	VkCommandBuffer command_buffer,
	game_engine::ObjectManagerSystem::ObjectMap& game_objects,
	const Camera& camera,
	const VkDescriptorSet global_descriptor_set,
	const VkDescriptorSet joint_descriptor_set,
	const VkDescriptorSet texture_descriptor_set
)
{
	pipeline->bind(command_buffer);

	std::vector<VkDescriptorSet> descriptor_sets{ global_descriptor_set, joint_descriptor_set, texture_descriptor_set };

	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout,
		0,
		descriptor_sets.size(),
		descriptor_sets.data(),
		0,
		nullptr
	);

	for (auto& obj : game_objects)
	{
		if (obj.second.gltf_model == nullptr) continue;
		for (auto& model : obj.second.gltf_model->models)
		{
			PushConstantData push{};
			if (model == nullptr) continue;
			push.model_matrix = obj.second.transform.matrix();
			push.color = obj.second.color;
			push.texture_index = obj.second.gltf_model->texture_id;
			
			vkCmdPushConstants(
				command_buffer,
				pipeline_layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PushConstantData),
				&push
			);
			model->bind(command_buffer);
			model->draw(command_buffer);
		}
	}
}

void game_engine::RenderSystem::render_wireframe_game_objects(
	VkCommandBuffer command_buffer,
	game_engine::ObjectManagerSystem::ObjectMap& game_objects,
	const Camera& camera,
	const VkDescriptorSet global_descriptor_set,
	const VkDescriptorSet joint_descriptor_set
)
{
	wireframe_pipeline->bind(command_buffer);

	std::vector<VkDescriptorSet> descriptor_sets{ global_descriptor_set, joint_descriptor_set };

	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		wireframe_pipeline_layout,
		0,
		descriptor_sets.size(),
		descriptor_sets.data(),
		0,
		nullptr
	);

	for (auto& obj : game_objects)
	{
		for (auto& model : obj.second.gltf_model->models)
		{
			PushConstantData push{};
			if (model == nullptr) continue;
			push.model_matrix = obj.second.transform.matrix();
			push.color = obj.second.color;
			push.texture_index = -1;  // No textures in wireframe mode
			
			vkCmdPushConstants(
				command_buffer,
				wireframe_pipeline_layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PushConstantData),
				&push
			);
			model->bind(command_buffer);
			model->draw(command_buffer);
		}
	}
}
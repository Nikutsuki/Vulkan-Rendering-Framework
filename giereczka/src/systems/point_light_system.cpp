#include "point_light_system.h"

game_engine::PointLightSystem::PointLightSystem(
	Device& device,
	VkRenderPass render_pass,
	VkDescriptorSetLayout global_set_layout
) : device(device)
{
	create_pipeline_layout(global_set_layout);
	create_pipeline(render_pass);
}

game_engine::PointLightSystem::~PointLightSystem()
{
	vkDestroyPipelineLayout(device.get_logical_device(), pipeline_layout, nullptr);
}

void game_engine::PointLightSystem::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PointLightPushConstants);

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts{ global_set_layout };

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

void game_engine::PointLightSystem::create_pipeline(VkRenderPass render_pass)
{
	assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
	pipeline_config_info pipeline_config{};
	Pipeline::default_pipeline_config_info(pipeline_config);
	pipeline_config.vertex_attribute_descriptions.clear();
	pipeline_config.vertex_binding_descriptions.clear();

	pipeline_config.render_pass = render_pass;
	pipeline_config.pipeline_layout = pipeline_layout;
	pipeline = std::make_unique<LightPipeline>(
		device,
		"compiled_shaders/point_light.vert.spv",
		"compiled_shaders/point_light.frag.spv",
		pipeline_config
	);
}

void game_engine::PointLightSystem::update(ObjectManagerSystem::PointLightMap& point_lights, game_engine::GlobalUbo& global_ubo, float dt)
{
	int light_index = 0;
	
	time += dt/2;

	float wave_speed = 1.0f;
	float wave_amplitude = 0.5f;

	for (auto& point_light : point_lights)
	{
		auto obj = point_light.second;
		global_ubo.point_lights[light_index].position = glm::vec4(obj.position, 1.f);
		global_ubo.point_lights[light_index].color = glm::vec4(obj.light_color, obj.light_intensity);

		++light_index;
	}

	global_ubo.num_point_lights = light_index;
}

void game_engine::PointLightSystem::render(
	VkCommandBuffer command_buffer,
	game_engine::ObjectManagerSystem::PointLightMap& point_lights,
	const Camera& camera,
	const VkDescriptorSet global_descriptor_set
)
{
	pipeline->bind(command_buffer);

	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout,
		0,
		1,
		&global_descriptor_set,
		0,
		nullptr
	);

	for (auto& point_light : point_lights)
	{
		auto& obj = point_light.second;
		PointLightPushConstants push_constants{};
		push_constants.light_position = glm::vec4(obj.position, 1.f);
		push_constants.light_color = glm::vec4(obj.light_color, obj.light_intensity);
		push_constants.light_radius = obj.light_radius;
		vkCmdPushConstants(
			command_buffer,
			pipeline_layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PointLightPushConstants),
			&push_constants
		);
		vkCmdDraw(command_buffer, 6, 1, 0, 0);
	}
}

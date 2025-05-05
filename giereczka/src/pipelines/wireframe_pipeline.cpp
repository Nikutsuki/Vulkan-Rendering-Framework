//
// Created by Niku on 3/8/2025.
//
#include "pipelines/wireframe_pipeline.h"

game_engine::WireframePipeline::WireframePipeline(Device &device, const std::string &vertex_shader_file_path,
                                                  const std::string &geometry_shader_file_path,
                                                  const std::string &fragment_shader_file_path,
                                                  const pipeline_config_info &config_info) : Pipeline(device)
{
	create_graphics_pipeline(vertex_shader_file_path, geometry_shader_file_path, fragment_shader_file_path, config_info);
}

void game_engine::WireframePipeline::create_graphics_pipeline(
	const std::string &vertex_shader_file_path,
	const std::string &geometry_shader_file_path,
	const std::string &fragment_shader_file_path,
	const pipeline_config_info &config_info
)
{
	assert(config_info.pipeline_layout != nullptr && "Cannot create graphics pipeline: no pipeline layout specified");
	assert(config_info.render_pass != nullptr && "Cannot create graphics pipeline: no render pass specified");

	auto vertex_shader_code = read_file(vertex_shader_file_path);
	auto geometry_shader_code = read_file(geometry_shader_file_path);
	auto fragment_shader_code = read_file(fragment_shader_file_path);

	std::cout << "Vertex shader code size: " << vertex_shader_code.size() << std::endl;
	std::cout << "Geometry shader code size: " << geometry_shader_code.size() << std::endl;
	std::cout << "Fragment shader code size: " << fragment_shader_code.size() << std::endl;

	if (vertex_shader_code.empty() || geometry_shader_code.empty() || fragment_shader_code.empty())
		throw std::runtime_error("Vertex, geometry or fragment shader code is empty");

	create_shader_module(vertex_shader_code, &vertex_shader_module);
	create_shader_module(geometry_shader_code, &geometry_shader_module);
	create_shader_module(fragment_shader_code, &fragment_shader_module);

	VkPipelineShaderStageCreateInfo shaderStages[3];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertex_shader_module;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
	shaderStages[1].module = geometry_shader_module;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[2].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[2].module = fragment_shader_module;
	shaderStages[2].pName = "main";
	shaderStages[2].flags = 0;
	shaderStages[2].pNext = nullptr;
	shaderStages[2].pSpecializationInfo = nullptr;

	auto &binding_descriptions = config_info.vertex_binding_descriptions;
	auto &attribute_descriptions = config_info.vertex_attribute_descriptions;

	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
	vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 3;
	pipeline_info.pStages = shaderStages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
	pipeline_info.pViewportState = &config_info.viewport_info;
	pipeline_info.pRasterizationState = &config_info.rasterization_info;
	pipeline_info.pMultisampleState = &config_info.multisample_info;
	pipeline_info.pColorBlendState = &config_info.color_blend_info;
	pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
	pipeline_info.pDynamicState = &config_info.dynamic_state_info;

	pipeline_info.layout = config_info.pipeline_layout;
	pipeline_info.renderPass = config_info.render_pass;
	pipeline_info.subpass = config_info.subpass;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(
		    device.get_logical_device(),
		    VK_NULL_HANDLE,
		    1,
		    &pipeline_info,
		    nullptr,
		    &graphics_pipeline
	    ) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}
}

#include "pipelines/pipeline.h"

game_engine::Pipeline::Pipeline(Device& device) : device(device)
{
}

game_engine::Pipeline::~Pipeline()
{
	vkDestroyPipeline(device.get_logical_device(), graphics_pipeline, nullptr);
}

void game_engine::Pipeline::bind(VkCommandBuffer command_buffer)
{
	if(graphics_pipeline == nullptr)
	{
		throw std::runtime_error("Graphics pipeline is nullptr");
	}
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
}

void game_engine::Pipeline::default_pipeline_config_info(pipeline_config_info& config_info)
{
	config_info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	config_info.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	config_info.input_assembly_info.primitiveRestartEnable = VK_FALSE;

	config_info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	config_info.viewport_info.viewportCount = 1;
	config_info.viewport_info.pViewports = nullptr;
	config_info.viewport_info.scissorCount = 1;
	config_info.viewport_info.pScissors = nullptr;

	config_info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	config_info.rasterization_info.depthClampEnable = VK_FALSE;
	config_info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
	config_info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
	config_info.rasterization_info.lineWidth = 1.0f;
	config_info.rasterization_info.cullMode = VK_CULL_MODE_NONE;
	config_info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	config_info.rasterization_info.depthBiasEnable = VK_FALSE;
	config_info.rasterization_info.depthBiasConstantFactor = 0.0f;
	config_info.rasterization_info.depthBiasClamp = 0.0f;
	config_info.rasterization_info.depthBiasSlopeFactor = 0.0f;

	config_info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	config_info.multisample_info.sampleShadingEnable = VK_FALSE;
	config_info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	config_info.multisample_info.minSampleShading = 1.0f;
	config_info.multisample_info.pSampleMask = nullptr;
	config_info.multisample_info.alphaToCoverageEnable = VK_FALSE;
	config_info.multisample_info.alphaToOneEnable = VK_FALSE;

	config_info.color_blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	config_info.color_blend_attachment.blendEnable = VK_FALSE;
	config_info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	config_info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	config_info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	config_info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	config_info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	config_info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	config_info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	config_info.color_blend_info.logicOpEnable = VK_FALSE;
	config_info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
	config_info.color_blend_info.attachmentCount = 1;
	config_info.color_blend_info.pAttachments = &config_info.color_blend_attachment;
	config_info.color_blend_info.blendConstants[0] = 0.0f;
	config_info.color_blend_info.blendConstants[1] = 0.0f;
	config_info.color_blend_info.blendConstants[2] = 0.0f;
	config_info.color_blend_info.blendConstants[3] = 0.0f;

	config_info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	config_info.depth_stencil_info.depthTestEnable = VK_TRUE;
	config_info.depth_stencil_info.depthWriteEnable = VK_TRUE;
	config_info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	config_info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	config_info.depth_stencil_info.minDepthBounds = 0.0f;
	config_info.depth_stencil_info.maxDepthBounds = 1.0f;
	config_info.depth_stencil_info.stencilTestEnable = VK_FALSE;
	config_info.depth_stencil_info.front = {};
	config_info.depth_stencil_info.back = {};

	config_info.dynamic_state_enables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	config_info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	config_info.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config_info.dynamic_state_enables.size());
	config_info.dynamic_state_info.pDynamicStates = config_info.dynamic_state_enables.data();
	config_info.dynamic_state_info.flags = 0;

	config_info.vertex_attribute_descriptions = Model::Vertex::get_attribute_descriptions();
	config_info.vertex_binding_descriptions = Model::Vertex::get_binding_descriptions();
}

std::vector<char> game_engine::Pipeline::read_file(const std::string& file_path)
{
	if (file_path == "none")
	{
		return std::vector<char>();
	}
	std::ifstream file{ file_path, std::ios::ate | std::ios::binary };

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + file_path);
	}

	size_t file_size = static_cast<size_t>(file.tellg());

	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();

	return buffer;
}

void game_engine::Pipeline::create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module)
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(device.get_logical_device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module");
	}
}

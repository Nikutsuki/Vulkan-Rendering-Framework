//
// Created by Niku on 3/1/2025.
//

#include "pipelines/raytracing_pipeline.h"

game_engine::RaytracingPipeline::RaytracingPipeline(Device &device, const std::string &compute_shader_file_path,
                                                      const pipeline_config_info &config_info) : Pipeline(device)
{
	create_graphics_pipeline(compute_shader_file_path, config_info);
}

void game_engine::RaytracingPipeline::create_graphics_pipeline(const std::string &compute_shader_file_path,
                                                                const pipeline_config_info &config_info)
{
	assert(config_info.pipeline_layout != nullptr && "Cannot create graphics pipeline: no pipeline layout specified");
	assert(config_info.render_pass != nullptr && "Cannot create graphics pipeline: no render pass specified");

	auto compute_shader_code = read_file(compute_shader_file_path);

	std::cout << "Compute shader code size: " << compute_shader_code.size() << std::endl;

	if (compute_shader_code.empty()) throw std::runtime_error("Compute shader code is empty");

	create_shader_module(compute_shader_code, &compute_shader_module);

	VkPipelineShaderStageCreateInfo compute_stage{};
	compute_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	compute_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	compute_stage.module = compute_shader_module;
	compute_stage.pName = "main";

	VkComputePipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.stage = compute_stage;
	pipeline_info.layout = config_info.pipeline_layout;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	if (vkCreateComputePipelines(
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

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>

#include "device.h"
#include "model.h"
#include "skeletal_animations/gltf_model.h"

namespace game_engine {
	struct pipeline_config_info {
		pipeline_config_info() = default;
		pipeline_config_info(const pipeline_config_info&) = delete;
		pipeline_config_info& operator=(const pipeline_config_info&) = delete;

		std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions;
		std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions;
		VkPipelineViewportStateCreateInfo viewport_info{};
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
		VkPipelineRasterizationStateCreateInfo rasterization_info{};
		VkPipelineMultisampleStateCreateInfo multisample_info{};
		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		VkPipelineColorBlendStateCreateInfo color_blend_info{};
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
		std::vector<VkDynamicState> dynamic_state_enables;
		VkPipelineDynamicStateCreateInfo dynamic_state_info{};
		VkPipelineLayout pipeline_layout = nullptr;
		VkRenderPass render_pass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline {
	public:
		Pipeline(Device& device);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		void bind(VkCommandBuffer command_buffer);

		static void default_pipeline_config_info(pipeline_config_info& config_info);
	protected:
		static std::vector<char> read_file(const std::string& file_path);

		void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

		Device& device;
		VkPipeline graphics_pipeline{};
	};
}
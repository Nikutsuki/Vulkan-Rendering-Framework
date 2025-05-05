//
// Created by Niku on 3/8/2025.
//
#pragma once

#include "pch.h"

#include "pipelines/pipeline.h"

namespace game_engine {
    class WireframePipeline : public Pipeline {
    public:
        WireframePipeline(Device& device,
            const std::string& vertex_shader_file_path,
            const std::string& geometry_shader_file_path,
            const std::string& fragment_shader_file_path,
            const pipeline_config_info& config_info
            );
    private:
        void create_graphics_pipeline(
            const std::string& vertex_shader_file_path,
            const std::string& geometry_shader_file_path,
            const std::string& fragment_shader_file_path,
            const pipeline_config_info& config_info
        );

        VkShaderModule vertex_shader_module;
        VkShaderModule geometry_shader_module;
        VkShaderModule fragment_shader_module;
    };
}
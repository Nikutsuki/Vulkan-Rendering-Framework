//
// Created by Niku on 3/1/2025.
//

#include "pch.h"

#include "pipeline.h"

namespace game_engine {
    class RaytracingPipeline : public Pipeline {
    public:
        RaytracingPipeline(Device& device,
            const std::string& compute_shader_file_path,
            const pipeline_config_info& config_info
            );
    private:
        void create_graphics_pipeline(
            const std::string& compute_shader_file_path,
            const pipeline_config_info& config_info
        );

        VkShaderModule compute_shader_module;
    };
}
#pragma once

#include "pipelines/light_pipeline.h"
#include "device.h"
#include "point_light_object.h"
#include "camera.h"
#include "global_ubo.h"
#include "object_manager_system.h"

#include <memory>
#include <vector>
#include <glm/gtc/constants.hpp>

namespace game_engine {
	struct PointLightPushConstants {
		glm::vec4 light_position{};
		glm::vec4 light_color{};
		float light_radius;
	};

	class PointLightSystem {
	public:
		PointLightSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(
			ObjectManagerSystem::PointLightMap& point_lights,
			GlobalUbo& global_ubo,
			float dt
		);

		void render(
			VkCommandBuffer command_buffer,
			game_engine::ObjectManagerSystem::PointLightMap& point_lights,
			const Camera& camera,
			const VkDescriptorSet global_descriptor_set
		);
	private:
		void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
		void create_pipeline(VkRenderPass render_pass);

		Device& device;

		std::unique_ptr<LightPipeline> pipeline;
		VkPipelineLayout pipeline_layout;

		float time = 0.f;
	};
}
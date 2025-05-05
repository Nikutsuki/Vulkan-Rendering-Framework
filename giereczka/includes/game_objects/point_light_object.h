#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace game_engine {
	class PointLightObject {
	public:
		struct PointLight {
			glm::vec4 position{};
			glm::vec4 color{};
		};
		PointLightObject() = default;
		PointLightObject(float intensity, float radius, glm::vec3 color, glm::vec3 position);
		void set_light_intensity(float intensity) { light_intensity = intensity; }
		void set_light_radius(float radius) { light_radius = radius; }
		void set_light_color(const glm::vec3& color) { light_color = color; }
		float light_intensity = 1.0f;
		float light_radius = 1.0f;
		glm::vec3 light_color = glm::vec3(1.0f);
		glm::vec3 position = glm::vec3(0.0f);
	};
}
#pragma once

#include "point_light_object.h"

#define MAX_LIGHTS 10

namespace game_engine {
	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverse_view{ 1.f };
		glm::vec4 ambient_light{ 1.0f, 1.0f, 1.0f, 0.02f };
		PointLightObject::PointLight point_lights[MAX_LIGHTS];
		alignas(16) int num_point_lights = 0;
	};
}
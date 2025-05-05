#pragma once

#include "skeletal_animations/gltf_model.h"

#include <memory>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

namespace game_engine {
	struct transform_component {
		glm::vec3 translation{};
		float scale = 1.0f;
		glm::vec3 rotation{};

		glm::mat4 matrix() const {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {
                    scale * (c1 * c3 + s1 * s2 * s3),
                    scale * (c2 * s3),
                    scale * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale * (c3 * s1 * s2 - c1 * s3),
                    scale * (c2 * c3),
                    scale * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale * (c2 * s1),
                    scale * (-s2),
                    scale * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}
            };
		}
	};

	class GameObject {
	public:
        GameObject();
        GameObject(
            std::shared_ptr<GltfModel> gltf_model,
            glm::vec3 color,
            glm::vec3 translation,
            float scale,
            glm::vec3 rotation,
            std::string name
        );

		std::shared_ptr<GltfModel> gltf_model;
		glm::vec3 color{};
		transform_component transform;
		std::string name;
	};
}
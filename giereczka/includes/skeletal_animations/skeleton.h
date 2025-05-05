#pragma once

#include "pch.h"

namespace game_engine {
	namespace Armature {
		static constexpr int NO_PARENT = -1;
		static constexpr int ROOT_JOINT = 0;

		struct ShaderData
		{
			std::vector<glm::mat4> final_joint_matrices;
		};

		struct Joint
		{
			std::string name;
			glm::mat4 inverse_bind_matrix;

			glm::vec3 deformed_position{0.0f};
			glm::quat deformed_rotation{1.0f, 0.0f, 0.0f, 0.0f};
			glm::vec3 deformed_scale{ 1.0f };

			glm::mat4 get_deformed_bind_matrix()
			{
				return glm::translate(glm::mat4(1.0f), deformed_position) *
					glm::mat4(deformed_rotation) *
					glm::scale(glm::mat4(1.0f), deformed_scale);
			}

			int parent_joint = NO_PARENT;
			std::vector<int> children;
		};

		struct Skeleton
		{
			void Traverse();
			void Traverse(Joint const& joint, uint32_t indent = 0);
			void Update();
			void UpdateJoint(int16_t joint_index);
			
			bool is_animated = true;
			std::string name;
			std::vector<Joint> joints;
			std::map<int, int> global_node_to_joint_index;
			ShaderData shader_data;
		};
	}
}
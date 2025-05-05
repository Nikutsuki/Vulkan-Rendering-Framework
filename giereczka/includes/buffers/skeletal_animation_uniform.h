#pragma once

#define MAX_JOINTS 100
#define MAX_WEIGHTS 4

namespace game_engine {
	struct JointUbo {
		glm::mat4 joint_matrices[MAX_JOINTS];
		alignas(16) int num_joints = 0;
	};
}
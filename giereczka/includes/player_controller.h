#pragma once

#include "game_object.h"
#include "camera.h"

namespace game_engine {
	class PlayerController : public GameObject {
	public:
		PlayerController();
		~PlayerController() = default;

		void move_player(glm::vec3 direction, float dt);
		void rotate_camera(double x_offset, double y_offset, float dt);
		Camera& get_camera() { return camera; }
	private:
		friend class DebugUI;
		Camera camera;
		float mouse_sensitivity = 0.1f;
		float movement_speed = 5.f;
	};
}
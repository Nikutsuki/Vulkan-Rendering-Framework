#pragma once

#include "player_controller.h"

namespace game_engine {
	class KeyboardController {
	public:
		KeyboardController();
		void handle_input(GLFWwindow* window, PlayerController& player_controller, float dt);
	private:
		struct KeyMappings {
			int move_left = GLFW_KEY_A;
			int move_right = GLFW_KEY_D;
			int move_forward = GLFW_KEY_W;
			int move_backward = GLFW_KEY_S;
			int move_up = GLFW_KEY_SPACE;
			int move_down = GLFW_KEY_LEFT_SHIFT;
			int open_debug_ui = GLFW_KEY_HOME;
		};
		KeyMappings key_mappings;

	protected:
		friend class TopazInputSystem;
	};

	class MouseController {
	public:
		MouseController();
		void handle_input(GLFWwindow* window, PlayerController& player_controller, float dt);
		static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);

		double x_movement = 0.0;
		double y_movement = 0.0;
	private:
	};

	class TopazInputSystem {
	public:
		TopazInputSystem(GLFWwindow* window, PlayerController& player_controller);
		void handle_input(float dt);

		bool get_is_menu_open() const { return is_menu_open; }
	private:
		KeyboardController keyboard_controller;
		MouseController mouse_controller;
		GLFWwindow* window;
		PlayerController& player_controller;
		bool is_menu_open = false;
		bool was_home_released = true;
	};
}
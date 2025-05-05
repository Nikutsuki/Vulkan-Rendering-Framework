#include "topaz_input_system.h"

game_engine::TopazInputSystem::TopazInputSystem(GLFWwindow* window, PlayerController& player_controller)
	: window(window),
	keyboard_controller(),
	mouse_controller(),
	player_controller(player_controller)
{
	glfwSetWindowUserPointer(window, &mouse_controller);
	glfwSetCursorPosCallback(window, MouseController::mouse_position_callback);
}

void game_engine::TopazInputSystem::handle_input(float dt)
{
	if (!is_menu_open)
	{
		keyboard_controller.handle_input(window, player_controller, dt);
		mouse_controller.handle_input(window, player_controller, dt);
	}
	
	if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS)
	{
		if (was_home_released)
		{
			was_home_released = false;
			if (!is_menu_open)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else
			{
				if (glfwRawMouseMotionSupported())
				{
					std::cout << "Raw mouse motion is supported" << std::endl;
					glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
				}
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

				mouse_controller.x_movement = 0.0;
				mouse_controller.y_movement = 0.0;
				glfwSetCursorPos(window, 0.0, 0.0);
			}
			is_menu_open = !is_menu_open;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_RELEASE)
	{
		was_home_released = true;
	}
}

game_engine::KeyboardController::KeyboardController()
{
}

void game_engine::KeyboardController::handle_input(GLFWwindow* window, PlayerController& player_controller, float dt)
{
	glm::vec3 direction = { 0.f, 0.f, 0.f };
	if (glfwGetKey(window, key_mappings.move_left) == GLFW_PRESS)
	{
		direction.x -= 1.f;
	}
	if (glfwGetKey(window, key_mappings.move_right) == GLFW_PRESS)
	{
		direction.x += 1.f;
	}
	if (glfwGetKey(window, key_mappings.move_forward) == GLFW_PRESS)
	{
		direction.z += 1.f;
	}
	if (glfwGetKey(window, key_mappings.move_backward) == GLFW_PRESS)
	{
		direction.z -= 1.f;
	}
	if (glfwGetKey(window, key_mappings.move_up) == GLFW_PRESS)
	{
		direction.y += 1.f;
	}
	if (glfwGetKey(window, key_mappings.move_down) == GLFW_PRESS)
	{
		direction.y -= 1.f;
	}

	player_controller.move_player(direction, dt);
}

game_engine::MouseController::MouseController()
{

}

void game_engine::MouseController::handle_input(GLFWwindow* window, PlayerController& player_controller, float dt)
{
	if (x_movement == 0.0 && y_movement == 0.0)
	{
		return;
	}
	player_controller.rotate_camera(x_movement, y_movement, dt);
	x_movement = 0.0;
	y_movement = 0.0;
	glfwSetCursorPos(window, 0.0, 0.0);
}

void game_engine::MouseController::mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	MouseController* mouse_controller = static_cast<MouseController*>(glfwGetWindowUserPointer(window));
	mouse_controller->x_movement += xpos;
	mouse_controller->y_movement -= ypos;
}

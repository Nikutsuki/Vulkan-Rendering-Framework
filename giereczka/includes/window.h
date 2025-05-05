#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>
#include <iostream>

namespace game_engine {
	class Window {
	public:
		Window(int width, int height, std::string name);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool should_close();
		VkExtent2D get_extent();
		bool was_window_resized();
		void reset_window_resized_flag();
		GLFWwindow* get_window() const { return window; }

		void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		static void frame_buffer_resize_callback(GLFWwindow* window, int width, int height);
		std::string name;

		int width;
		int height;
		bool framebuffer_resized = false;

		void initWindow();

		GLFWwindow* window;
	};
}
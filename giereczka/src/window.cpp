#include "window.h"

game_engine::Window::Window(int width, int height, std::string name) : width(width), height(height), name(name)
{
	initWindow();
}

game_engine::Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool game_engine::Window::should_close()
{
	return glfwWindowShouldClose(window);
}

VkExtent2D game_engine::Window::get_extent()
{
	return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

bool game_engine::Window::was_window_resized()
{
	return framebuffer_resized;
}

void game_engine::Window::reset_window_resized_flag()
{
	framebuffer_resized = false;
}

void game_engine::Window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface");
	}
}

void game_engine::Window::frame_buffer_resize_callback(GLFWwindow* window, int width, int height)
{
	auto my_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	my_window->framebuffer_resized = true;
	my_window->width = width;
	my_window->height = height;
}

void game_engine::Window::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (glfwRawMouseMotionSupported())
	{
		std::cout << "Raw mouse motion is supported" << std::endl;
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);
}

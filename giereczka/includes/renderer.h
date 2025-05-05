#pragma once

#include "window.h"
#include "device.h"
#include "swapchain.h"

#include <memory>
#include <vector>
#include <cassert>

namespace game_engine {
	class Renderer {
	public:
		Renderer(Window& window, Device& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass get_swap_chain_render_pass() const;
		float get_aspect_ratio() const;
		bool is_frame_in_progress() const;

		VkCommandBuffer get_current_command_buffer() const;

		int get_frame_index() const;

		VkCommandBuffer begin_frame();

		void end_frame();
		void begin_swap_chain_render_pass(VkCommandBuffer command_buffer);
		void end_swap_chain_render_pass(VkCommandBuffer command_buffer);

		std::unique_ptr<SwapChain> swap_chain;
	private:
		void create_command_buffers();
		void free_command_buffers();
		void recreate_swap_chain();

		Window& window;
		Device& device;
		std::vector<VkCommandBuffer> command_buffers;

		uint32_t current_image_index = 0;
		int current_frame_index = 0;
		bool is_frame_started = false;
	};
}
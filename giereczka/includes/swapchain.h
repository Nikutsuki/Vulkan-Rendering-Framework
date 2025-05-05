#pragma once

#include "device.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <array>

namespace game_engine {
	class SwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		SwapChain(Device& device, VkExtent2D window_extent);
		SwapChain(Device& device, VkExtent2D window_extent, std::shared_ptr<SwapChain> old_swap_chain);
		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		VkFramebuffer get_framebuffer(size_t index);
		VkRenderPass get_render_pass();
		VkImageView get_image_view(size_t index);
		size_t image_count();
		VkFormat get_swap_chain_image_format();
		VkExtent2D get_swap_chain_extent();
		uint32_t width();
		uint32_t height();

		float extent_aspect_ratio();

		VkFormat find_depth_format();
		VkResult acquire_next_image(uint32_t* image_index);
		VkResult submit_command_buffers(const VkCommandBuffer* buffers, uint32_t* image_index);

		bool compare_swap_formats(const SwapChain& swap_chain) const;
	private:
		void init();
		void create_swap_chain();
		void create_image_views();
		void create_render_pass();
		void create_depth_resources();
		void create_framebuffers();
		void create_sync_objects();

		VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
		VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat swap_chain_image_format;
		VkFormat swap_chain_depth_format;
		VkExtent2D swap_chain_extent;

		std::vector<VkFramebuffer> swap_chain_framebuffers;
		VkRenderPass render_pass;

		std::vector<VkImage> depth_images;
		std::vector<VkDeviceMemory> depth_image_memories;
		std::vector<VkImageView> depth_image_views;
		std::vector<VkImage> swap_chain_images;
		std::vector<VkImageView> swap_chain_image_views;

		Device& device;
		VkExtent2D window_extent;

		VkSwapchainKHR swap_chain;
		std::shared_ptr<SwapChain> old_swap_chain;

		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> in_flight_fences;
		std::vector<VkFence> images_in_flight;
		size_t current_frame = 0;
	};
}
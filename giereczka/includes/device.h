#pragma once

#include "window.h"

#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <unordered_set>

namespace game_engine {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	struct QueueFamilyIndices {
		uint32_t graphics_family;
		uint32_t present_family;
		bool graphics_family_has_value = false;
		bool present_family_has_value = false;
		bool is_complete() { return graphics_family_has_value && present_family_has_value; }
	};

	class Device {
	public:
#ifdef NDEBUG
		const bool enable_validation_layers = false;
#else
		const bool enable_validation_layers = true;
#endif
		Device(Window& window);
		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&&) = delete;

		VkCommandPool get_command_pool();
		VkDevice get_logical_device();
		VkSurfaceKHR get_surface();
		VkQueue get_graphics_queue();
		VkQueue get_present_queue();

		VkInstance get_instance();
		VkPhysicalDevice get_physical_device() { return physical_device; }

		SwapChainSupportDetails get_swap_chain_support();
		uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices find_physical_queue_families();
		VkFormat find_supported_format(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features);

		void create_buffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory
		);
		VkCommandBuffer begin_single_time_commands();
		void end_single_time_commands(VkCommandBuffer command_buffer);
		void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
		void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);
		void create_image_with_info(
			const VkImageCreateInfo& image_info,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& image_memory
		);
		VkPhysicalDeviceProperties properties;

	private:
		void create_instance();
		void setup_debug_messenger();
		void create_surface();
		void pick_physical_device();
		void create_logical_device();
		void create_command_pool();

		bool is_device_suitable(VkPhysicalDevice device);
		std::vector<const char*> get_required_extensions();
		bool check_validation_layer_support();
		QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
		void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
		void has_glfw_required_instance_extensions();
		bool check_device_extension_support(VkPhysicalDevice device);
		SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
		bool check_descriptor_indexing_support(VkPhysicalDevice device);

		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		Window& window;
		VkCommandPool command_pool;

		VkDevice device_;
		VkSurfaceKHR surface_;
		VkQueue graphics_queue;
		VkQueue present_queue;

		const std::vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> device_extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
		};
	};
}
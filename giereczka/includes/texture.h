#pragma once

#include "pch.h"

#include "vulkan/vulkan.h"

#include "device.h"
#include "stb_image.h"
#include <buffer.h>

namespace game_engine {
	class Texture {
	public:
		Texture(Device& device, bool nearest_filter = false);
		~Texture();
		bool init(const uint32_t width, const uint32_t height, bool sRGB, const void* data, int min_filter, int mag_filter);
		bool init(const std::string& file_name, bool sRGB, bool flip = true);
		bool init(const unsigned char* data, int length, bool sRGB);
		int get_width() const { return width; };
		int get_height() const { return height; };
		void resize(const uint32_t width, const uint32_t height);
		void blit(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bytes_per_pixel, const void* data);
		void blit(uint32_t x, uint32_t y, uint32_t width, uint32_t height, int data_format, int type, const void* data);
		void set_file_name(const std::string& file_name) { this->file_name = file_name; };
		std::string get_file_name() { return this->file_name; };

		VkDescriptorImageInfo& get_descriptor_image_info() { return descriptor_image_info; };
		VkImageView get_image_view() const { return texture_image_view; };
		VkSampler get_sampler() const { return texture_sampler; };

		bool create();

		static constexpr bool USE_SRGB = true;
		static constexpr bool USE_UNORM = false;
	private:
		void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
		void create_image(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		void transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout);
		void generate_mipmaps();

		VkFilter set_filter(int min_mag_filter);
		VkFilter set_filter_mip(int min_filter);

		Device& device;
		std::unique_ptr<Buffer> buffer;

		std::string file_name;
		unsigned char* local_buffer;
		int width;
		int height;
		int bytes_per_pixel;
		uint32_t mip_levels;

		bool sRGB;
		VkFilter min_filter;
		VkFilter mag_filter;
		VkFilter min_filter_mip;

		VkFormat image_format{ VkFormat::VK_FORMAT_UNDEFINED };
		VkImage texture_image{ nullptr };
		VkDeviceMemory texture_image_memory{ nullptr };
		VkImageLayout image_layout{ VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageView texture_image_view{ nullptr };
		VkSampler texture_sampler{ nullptr };

		VkDescriptorImageInfo descriptor_image_info{};
	};
}
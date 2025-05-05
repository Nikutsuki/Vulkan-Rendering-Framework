#pragma once

#include "device.h"

#include <cassert>

namespace game_engine {
	class Buffer {
	public:
		Buffer(
			Device& device,
			VkDeviceSize instance_size,
			uint32_t instance_count,
			VkBufferUsageFlags usage_flags,
			VkMemoryPropertyFlags memory_property_flags,
			VkDeviceSize min_offset_alignment = 1
		);
		~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();

		void write_to_buffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkDescriptorBufferInfo descriptor_info(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void write_to_index(void* data, int index);
		VkResult flush_index(int index);
		VkDescriptorBufferInfo descriptor_info_index(int index) const;
		VkResult invalidate_index(int index);

		VkBuffer get_buffer() const { return buffer; }
		void* get_mapped_memory() const { return mapped_memory; }
		uint32_t get_instance_count() const { return instance_count; }
		VkDeviceSize get_instance_size() const { return instance_size; }
		VkDeviceSize get_alignment_size() const { return alignment_size; }
		VkBufferUsageFlags get_usage_flags() const { return usage_flags; }
		VkMemoryPropertyFlags get_memory_property_flags() const { return memory_property_flags; }
		VkDeviceSize get_buffer_size() const { return buffer_size; }

	private:
		static VkDeviceSize get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

		Device& device;
		void* mapped_memory = nullptr;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
		
		VkDeviceSize buffer_size;
		uint32_t instance_count;
		VkDeviceSize instance_size;
		VkDeviceSize alignment_size;
		VkBufferUsageFlags usage_flags;
		VkMemoryPropertyFlags memory_property_flags;
	};
}
#include "buffer.h"

VkDeviceSize game_engine::Buffer::get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment)
{
	if (min_offset_alignment > 0)
	{
		return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
	}

	return instance_size;
}

game_engine::Buffer::Buffer(Device& device, VkDeviceSize instance_size, uint32_t instance_count, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, VkDeviceSize min_offset_alignment)
	: device{device},
	instance_size{instance_size},
	instance_count{ instance_count },
	usage_flags{ usage_flags },
	memory_property_flags{ memory_property_flags }
{
	alignment_size = get_alignment(instance_size, min_offset_alignment);
	buffer_size = alignment_size * instance_count;
	device.create_buffer(buffer_size, usage_flags, memory_property_flags, buffer, buffer_memory);
}

game_engine::Buffer::~Buffer()
{
	unmap();
	vkDestroyBuffer(device.get_logical_device(), buffer, nullptr);
	vkFreeMemory(device.get_logical_device(), buffer_memory, nullptr);
}

VkResult game_engine::Buffer::map(VkDeviceSize size, VkDeviceSize offset)
{
	assert(buffer && buffer_memory && "Cannot map memory before buffer has been created");

	return vkMapMemory(device.get_logical_device(), buffer_memory, offset, size, 0, &mapped_memory);
}

void game_engine::Buffer::unmap()
{
	if (mapped_memory)
	{
		vkUnmapMemory(device.get_logical_device(), buffer_memory);
		mapped_memory = nullptr;
	}
}

void game_engine::Buffer::write_to_buffer(void* data, VkDeviceSize size, VkDeviceSize offset)
{
	assert(mapped_memory && "Cannot copy to memory before memory has been mapped");

	if (size == VK_WHOLE_SIZE)
	{
		memcpy(mapped_memory, data, static_cast<size_t>(buffer_size));
	}
	else
	{
		char* mem_offset = (char*)mapped_memory;
		mem_offset += offset;
		memcpy(mem_offset, data, static_cast<size_t>(size));
	}
}

VkResult game_engine::Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mapped_range = {};
	mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped_range.memory = buffer_memory;
	mapped_range.offset = offset;
	mapped_range.size = size;
	return vkFlushMappedMemoryRanges(device.get_logical_device(), 1, &mapped_range);
}

VkDescriptorBufferInfo game_engine::Buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset) const
{
	return VkDescriptorBufferInfo{
		buffer,
		offset,
		size
	};
}

VkResult game_engine::Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mapped_range = {};
	mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped_range.memory = buffer_memory;
	mapped_range.offset = offset;
	mapped_range.size = size;
	return vkInvalidateMappedMemoryRanges(device.get_logical_device(), 1, &mapped_range);
}

void game_engine::Buffer::write_to_index(void* data, int index)
{
	write_to_buffer(data, instance_size, index * alignment_size);
}

VkResult game_engine::Buffer::flush_index(int index)
{
	return flush(instance_size, index * alignment_size);
}

VkDescriptorBufferInfo game_engine::Buffer::descriptor_info_index(int index) const
{
	return descriptor_info(instance_size, index * alignment_size);
}

VkResult game_engine::Buffer::invalidate_index(int index)
{
	return invalidate(alignment_size, index * alignment_size);
}

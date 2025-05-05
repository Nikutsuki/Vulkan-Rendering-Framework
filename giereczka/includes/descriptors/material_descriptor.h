#pragma once

#include "pch.h"

#include "material.h"
#include "descriptors.h"

#define MAX_TEXTURES 1024

namespace game_engine {
	class MaterialDescriptor {
	public:
		MaterialDescriptor(Device& device, std::unique_ptr<DescriptorPool>& descriptor_pool);

		void write_texture(VkImageView image_view, VkSampler sampler, uint32_t index);

		const VkDescriptorSet& get_descriptor_set() const { return descriptor_set; }
		const VkDescriptorSetLayout get_descriptor_set_layout() const { return descriptor_set_layout; }
	private:
		Device& device;
		VkDescriptorSetLayout descriptor_set_layout{};
		VkDescriptorSet descriptor_set{};
		VkDescriptorSetLayoutBinding textures_binding{};
	};
}
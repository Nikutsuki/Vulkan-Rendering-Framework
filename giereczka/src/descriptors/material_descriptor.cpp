#include "descriptors/material_descriptor.h"

game_engine::MaterialDescriptor::MaterialDescriptor(Device& device, std::unique_ptr<DescriptorPool>& descriptor_pool) : device{device}
{
    textures_binding.binding = 0;
    textures_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textures_binding.descriptorCount = MAX_TEXTURES; // Can be a large number like 1024
    textures_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // VkDescriptorBindingFlags bindingFlag =
    //     VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
    //     VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    //
    // VkDescriptorSetLayoutBindingFlagsCreateInfo bindings_info{};
    // bindings_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    // bindings_info.bindingCount = 1;
    // bindings_info.pBindingFlags = &bindingFlag;

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &textures_binding;
    //layout_info.pNext = &bindings_info;
    layout_info.pNext = nullptr;
    layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    if (vkCreateDescriptorSetLayout(device.get_logical_device(), &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool->get_descriptor_pool();
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set_layout;

    if (vkAllocateDescriptorSets(device.get_logical_device(), &alloc_info, &descriptor_set) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }
}

void game_engine::MaterialDescriptor::write_texture(VkImageView image_view, VkSampler sampler, uint32_t index)
{
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    VkWriteDescriptorSet write_descriptor{};
    write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor.dstSet = descriptor_set;
    write_descriptor.dstBinding = 0;
    write_descriptor.dstArrayElement = index;
    write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor.descriptorCount = 1;
    write_descriptor.pImageInfo = &image_info;

    vkUpdateDescriptorSets(device.get_logical_device(), 1, &write_descriptor, 0, nullptr);
}

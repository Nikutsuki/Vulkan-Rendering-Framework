#include "descriptors.h"

game_engine::DescriptorPool::Builder& game_engine::DescriptorPool::Builder::add_pool_size(VkDescriptorType type, uint32_t count)
{
	pool_sizes.push_back({ type, count });
	return *this;
}

game_engine::DescriptorPool::Builder& game_engine::DescriptorPool::Builder::set_pool_flags(VkDescriptorPoolCreateFlags flags)
{
	pool_flags = flags;
	return *this;
}

game_engine::DescriptorPool::Builder& game_engine::DescriptorPool::Builder::set_max_sets(uint32_t count)
{
	max_sets = count;
	return *this;
}

game_engine::DescriptorSetLayout::Builder& game_engine::DescriptorSetLayout::Builder::add_binding(
	uint32_t binding,
	VkDescriptorType descriptor_type,
	VkShaderStageFlags stage_flags,
	uint32_t count
	)
{
	assert(bindings.count(binding) == 0 && "Binding already in use");
	VkDescriptorSetLayoutBinding layout_binding{};
	layout_binding.binding = binding;
	layout_binding.descriptorType = descriptor_type;
	layout_binding.descriptorCount = count;
	layout_binding.stageFlags = stage_flags;
	layout_binding.pImmutableSamplers = nullptr;
	bindings[binding] = layout_binding;

	return *this;
}

std::unique_ptr<game_engine::DescriptorSetLayout> game_engine::DescriptorSetLayout::Builder::build() const
{
	return std::make_unique<DescriptorSetLayout>(device, bindings);
}

game_engine::DescriptorSetLayout::DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : device{ device }, bindings{ bindings }
{
	std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
	for (auto kv : bindings)
	{
		set_layout_bindings.push_back(kv.second);
	}

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
	descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
	descriptor_set_layout_create_info.pBindings = set_layout_bindings.data();

	if (vkCreateDescriptorSetLayout(
		device.get_logical_device(),
		&descriptor_set_layout_create_info,
		nullptr,
		&descriptor_set_layout
	) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}
}

std::unique_ptr<game_engine::DescriptorPool> game_engine::DescriptorPool::Builder::build() const {
	return std::make_unique<DescriptorPool>(device, max_sets, pool_flags, pool_sizes);
}

game_engine::DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device.get_logical_device(), descriptor_set_layout, nullptr);
}

game_engine::DescriptorPool::DescriptorPool(Device& device, uint32_t max_sets, VkDescriptorPoolCreateFlags pool_flags, const std::vector<VkDescriptorPoolSize>& pool_sizes) : device{device}
{
	VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	descriptor_pool_create_info.pPoolSizes = pool_sizes.data();
	descriptor_pool_create_info.maxSets = max_sets;
	descriptor_pool_create_info.flags = pool_flags;

	if (vkCreateDescriptorPool(
		device.get_logical_device(),
		&descriptor_pool_create_info,
		nullptr,
		&descriptor_pool
	) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

game_engine::DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(device.get_logical_device(), descriptor_pool, nullptr);
}

bool game_engine::DescriptorPool::allocate_descriptor(const VkDescriptorSetLayout layout, VkDescriptorSet& descriptor) const
{
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
	descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_allocate_info.descriptorPool = descriptor_pool;
	descriptor_set_allocate_info.descriptorSetCount = 1;
	descriptor_set_allocate_info.pSetLayouts = &layout;

	return vkAllocateDescriptorSets(
		device.get_logical_device(),
		&descriptor_set_allocate_info,
		&descriptor
	) == VK_SUCCESS;
}

void game_engine::DescriptorPool::free_descriptors(const std::vector<VkDescriptorSet>& descriptors) const
{
	vkFreeDescriptorSets(
		device.get_logical_device(),
		descriptor_pool,
		static_cast<uint32_t>(descriptors.size()),
		descriptors.data()
	);
}

void game_engine::DescriptorPool::reset_pool()
{
	vkResetDescriptorPool(device.get_logical_device(), descriptor_pool, 0);
}

game_engine::DescriptorWriter::DescriptorWriter(DescriptorSetLayout& set_layout, DescriptorPool& pool) : set_layout{ set_layout }, pool{ pool }
{
}

game_engine::DescriptorWriter& game_engine::DescriptorWriter::write_buffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info)
{
	assert(set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto& binding_description = set_layout.bindings[binding];

	assert(
		binding_description.descriptorCount == 1 &&
		"Binding single descriptor info, but binding expects multiple"
	);

	VkWriteDescriptorSet write_descriptor_set{};

	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.descriptorType = binding_description.descriptorType;
	write_descriptor_set.dstBinding = binding;
	write_descriptor_set.pBufferInfo = buffer_info;
	write_descriptor_set.descriptorCount = 1;

	writes.push_back(write_descriptor_set);
	return *this;
}

game_engine::DescriptorWriter& game_engine::DescriptorWriter::write_image(uint32_t binding, VkDescriptorImageInfo* image_info)
{
	assert(set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto& binding_description = set_layout.bindings[binding];

	assert(
		binding_description.descriptorCount == 1 &&
		"Binding single descriptor info, but binding expects multiple"
	);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = binding_description.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = image_info;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

bool game_engine::DescriptorWriter::build(VkDescriptorSet& set)
{
	if (set == VK_NULL_HANDLE)
	{
		bool success = pool.allocate_descriptor(set_layout.descriptor_set_layout, set);
		if (!success)
		{
			return false;
		}
	}

	overwrite(set);
	return true;
}

void game_engine::DescriptorWriter::overwrite(VkDescriptorSet set)
{
	for (auto& write : writes)
	{
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(
		pool.device.get_logical_device(),
		static_cast<uint32_t>(writes.size()),
		writes.data(),
		0,
		nullptr
	);
}

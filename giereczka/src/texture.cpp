#include "texture.h"

game_engine::Texture::Texture(Device& device, bool nearest_filter) : device{device}, file_name(""), local_buffer(nullptr), width(0), height(0), bytes_per_pixel(0), mip_levels(0), sRGB(false)
{
	nearest_filter ? min_filter = VK_FILTER_NEAREST : min_filter = VK_FILTER_LINEAR;
	nearest_filter ? mag_filter = VK_FILTER_NEAREST : mag_filter = VK_FILTER_LINEAR;
	min_filter_mip = VK_FILTER_LINEAR;
}

game_engine::Texture::~Texture()
{
	vkDestroyImage(device.get_logical_device(), texture_image, nullptr);
	vkDestroyImageView(device.get_logical_device(), texture_image_view, nullptr);
	vkDestroySampler(device.get_logical_device(), texture_sampler, nullptr);
	vkFreeMemory(device.get_logical_device(), texture_image_memory, nullptr);
}

bool game_engine::Texture::init(const uint32_t width, const uint32_t height, bool sRGB, const void* data, int min_filter, int mag_filter)
{
	bool ok = false;
	file_name = "raw memory";
	this->sRGB = sRGB;
	local_buffer = (unsigned char*)data;
	this->min_filter = set_filter(min_filter);
	this->mag_filter = set_filter(mag_filter);
	this->min_filter_mip = set_filter_mip(min_filter);

	if (local_buffer)
	{
		this->width = width;
		this->height = height;
		bytes_per_pixel = 4;
		ok = create();
	}

	return ok;
}

bool game_engine::Texture::init(const std::string& file_name, bool sRGB, bool flip)
{
	bool ok = false;
	stbi_set_flip_vertically_on_load(flip);
	this->file_name = file_name;
	this->sRGB = sRGB;
	local_buffer = stbi_load(file_name.c_str(), &width, &height, &bytes_per_pixel, STBI_rgb_alpha);

	if (local_buffer)
	{
		ok = create();
		stbi_image_free(local_buffer);
	}
	else
	{
		throw std::runtime_error("Failed to load texture image: " + file_name);
	}
	return ok;
}

bool game_engine::Texture::init(const unsigned char* data, int length, bool sRGB)
{
	bool ok = false;
	stbi_set_flip_vertically_on_load(true);
	this->file_name = "file in memory";
	this->sRGB = sRGB;
	local_buffer = stbi_load_from_memory(data, length, &width, &height, &bytes_per_pixel, STBI_rgb_alpha);

	if (local_buffer)
	{
		ok = create();
		stbi_image_free(local_buffer);
	}
	else
	{
		throw std::runtime_error("Failed to load texture image from memory");
	}
	return ok;
}

void game_engine::Texture::transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout)
{
	VkCommandBuffer command_buffer = device.begin_single_time_commands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = texture_image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument("Unsupported layout transition");
		return;
	}
	
	vkCmdPipelineBarrier(
		command_buffer,
		source_stage, destination_stage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	device.end_single_time_commands(command_buffer);
}

void game_engine::Texture::create_image(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
	mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	this->image_format = format;

	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = mip_levels;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	{
		auto result = vkCreateImage(device.get_logical_device(), &image_info, nullptr, &texture_image);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image");
		}
	}

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device.get_logical_device(), texture_image, &memory_requirements);

	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memory_requirements.size;
	alloc_info.memoryTypeIndex = device.find_memory_type(memory_requirements.memoryTypeBits, properties);
	{
		auto result = vkAllocateMemory(device.get_logical_device(), &alloc_info, nullptr, &texture_image_memory);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory");
		}
	}

	vkBindImageMemory(device.get_logical_device(), texture_image, texture_image_memory, 0);
}

void game_engine::Texture::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
{
	// TODO
}

bool game_engine::Texture::create()
{
	VkDeviceSize image_size = width * height * bytes_per_pixel;

	if (!local_buffer)
	{
		throw std::runtime_error("Failed to load texture image");
		return false;
	}

	Buffer staging_buffer{
		device,
		image_size,
		1,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	staging_buffer.map();
	staging_buffer.write_to_buffer(local_buffer);

	VkFormat format = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

	create_image(
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	device.copy_buffer_to_image(
		staging_buffer.get_buffer(),
		texture_image,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height),
		1
	);

	generate_mipmaps();

	image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkSamplerCreateInfo sampler_create_info{};
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.magFilter = mag_filter;
	sampler_create_info.minFilter = min_filter;
	sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
	sampler_create_info.mipLodBias = 0.0f;
	sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_create_info.minLod = 0.0f;
	sampler_create_info.maxLod = static_cast<float>(mip_levels);
	sampler_create_info.maxAnisotropy = 4.0f;
	sampler_create_info.anisotropyEnable = VK_TRUE;
	sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	{
		auto result = vkCreateSampler(device.get_logical_device(), &sampler_create_info, nullptr, &texture_sampler);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler");
		}
	}

	VkImageViewCreateInfo view_info{};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	view_info.subresourceRange.levelCount = mip_levels;

	view_info.image = texture_image;

	{
		auto result = vkCreateImageView(device.get_logical_device(), &view_info, nullptr, &texture_image_view);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view");
		}
	}

	descriptor_image_info.imageLayout = image_layout;
	descriptor_image_info.imageView = texture_image_view;
	descriptor_image_info.sampler = texture_sampler;

	return true;
}

void game_engine::Texture::blit(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bytes_per_pixel, const void* data)
{
	throw std::runtime_error("Not implemented");
}

void game_engine::Texture::blit(uint32_t x, uint32_t y, uint32_t width, uint32_t height, int data_format, int type, const void* data)
{
	throw std::runtime_error("Not implemented");
}

void game_engine::Texture::resize(const uint32_t width, const uint32_t height)
{
	throw std::runtime_error("Not implemented");
}

void game_engine::Texture::generate_mipmaps()
{
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(device.get_physical_device(), image_format, &format_properties);

	if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("Texture image format does not support linear blitting");
		return;
	}

	VkCommandBuffer command_buffer = device.begin_single_time_commands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = texture_image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mip_width = width;
	int32_t mip_height = height;

	for (uint32_t i = 1; i < mip_levels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mip_width, mip_height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = {
			mip_width > 1 ? mip_width / 2 : 1,
			mip_height > 1 ? mip_height / 2 : 1,
			1
		};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			command_buffer,
			texture_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			min_filter_mip
		);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		if (mip_width > 1) mip_width /= 2;
		if (mip_height > 1) mip_height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	device.end_single_time_commands(command_buffer);
}

VkFilter game_engine::Texture::set_filter(int min_mag_filter)
{
	VkFilter filter = VK_FILTER_LINEAR;
	switch (min_mag_filter)
	{
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		filter = VK_FILTER_NEAREST;
		break;
	}
	return filter;
}

VkFilter game_engine::Texture::set_filter_mip(int min_filter)
{
	VkFilter filter = VK_FILTER_LINEAR;
	switch (min_filter)
	{
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	{
		break;
	}
	case GL_LINEAR_MIPMAP_NEAREST:
	{
		break;
	}
	{
		filter = VK_FILTER_NEAREST;
		break;
	}
	}
	return filter;
}
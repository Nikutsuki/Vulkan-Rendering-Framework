#include "swapchain.h"

game_engine::SwapChain::SwapChain(Device& device, VkExtent2D window_extent) : device(device), window_extent(window_extent)
{
	init();
}

game_engine::SwapChain::SwapChain(Device& device, VkExtent2D window_extent, std::shared_ptr<SwapChain> old_swap_chain)
	: device(device), window_extent(window_extent), old_swap_chain(old_swap_chain)
{
	init();

	old_swap_chain = nullptr;
}

game_engine::SwapChain::~SwapChain()
{
	for (auto image_view : swap_chain_image_views)
	{
		vkDestroyImageView(device.get_logical_device(), image_view, nullptr);
	}
	swap_chain_image_views.clear();

	if (swap_chain != nullptr)
	{
		vkDestroySwapchainKHR(device.get_logical_device(), swap_chain, nullptr);
		swap_chain = nullptr;
	}

	for (int i = 0; i < depth_images.size(); i++)
	{
		vkDestroyImageView(device.get_logical_device(), depth_image_views[i], nullptr);
		vkDestroyImage(device.get_logical_device(), depth_images[i], nullptr);
		vkFreeMemory(device.get_logical_device(), depth_image_memories[i], nullptr);
	}

	for (size_t i = 0; i < swap_chain_framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(device.get_logical_device(), swap_chain_framebuffers[i], nullptr);
	}

	vkDestroyRenderPass(device.get_logical_device(), render_pass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device.get_logical_device(), render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(device.get_logical_device(), image_available_semaphores[i], nullptr);
		vkDestroyFence(device.get_logical_device(), in_flight_fences[i], nullptr);
	}
}

VkFramebuffer game_engine::SwapChain::get_framebuffer(size_t index)
{
	return swap_chain_framebuffers[index];
}

VkRenderPass game_engine::SwapChain::get_render_pass()
{
	return render_pass;
}

VkImageView game_engine::SwapChain::get_image_view(size_t index)
{
	return swap_chain_image_views[index];
}

size_t game_engine::SwapChain::image_count()
{
	return swap_chain_images.size();
}

VkFormat game_engine::SwapChain::get_swap_chain_image_format()
{
	return swap_chain_image_format;
}

VkExtent2D game_engine::SwapChain::get_swap_chain_extent()
{
	return swap_chain_extent;
}

uint32_t game_engine::SwapChain::width()
{
	return swap_chain_extent.width;
}

uint32_t game_engine::SwapChain::height()
{
	return swap_chain_extent.height;
}

float game_engine::SwapChain::extent_aspect_ratio()
{
	return static_cast<float>(swap_chain_extent.width) / static_cast<float>(swap_chain_extent.height);
}

VkFormat game_engine::SwapChain::find_depth_format()
{
	return device.find_supported_format(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkResult game_engine::SwapChain::acquire_next_image(uint32_t* image_index)
{
	vkWaitForFences(
		device.get_logical_device(),
		1,
		&in_flight_fences[current_frame],
		VK_TRUE,
		std::numeric_limits<uint64_t>::max()
	);

	VkResult result = vkAcquireNextImageKHR(
		device.get_logical_device(),
		swap_chain,
		std::numeric_limits<uint64_t>::max(),
		image_available_semaphores[current_frame],
		VK_NULL_HANDLE,
		image_index
	);

	return result;
}

VkResult game_engine::SwapChain::submit_command_buffers(const VkCommandBuffer* buffers, uint32_t* image_index)
{
	if (images_in_flight[*image_index] != VK_NULL_HANDLE)
	{
		vkWaitForFences(
			device.get_logical_device(),
			1,
			&images_in_flight[*image_index],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max()
		);
	}

	images_in_flight[*image_index] = in_flight_fences[current_frame];

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { image_available_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = buffers;

	VkSemaphore signal_semaphores[] = { render_finished_semaphores[current_frame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	vkResetFences(device.get_logical_device(), 1, &in_flight_fences[current_frame]);
	if (vkQueueSubmit(
		device.get_graphics_queue(),
		1,
		&submit_info,
		in_flight_fences[current_frame]
	) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffer");
	}

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swap_chains[] = { swap_chain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;

	present_info.pImageIndices = image_index;

	VkResult result = vkQueuePresentKHR(device.get_present_queue(), &present_info);

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	return result;
}

bool game_engine::SwapChain::compare_swap_formats(const SwapChain& swap_chain) const
{
	return swap_chain.swap_chain_depth_format == swap_chain_depth_format
		&& swap_chain.swap_chain_image_format == swap_chain_image_format;
}

void game_engine::SwapChain::init()
{
	create_swap_chain();
	create_image_views();
	create_render_pass();
	create_depth_resources();
	create_framebuffers();
	create_sync_objects();
}

void game_engine::SwapChain::create_swap_chain()
{
	SwapChainSupportDetails swap_chain_support = device.get_swap_chain_support();

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
	VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
	VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);

	uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount)
	{
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info{};

	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = device.get_surface();

	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = device.find_physical_queue_families();
	uint32_t queue_family_indices[] = { indices.graphics_family, indices.present_family };

	if (indices.graphics_family != indices.present_family)
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	}

	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	create_info.oldSwapchain = old_swap_chain == nullptr ? VK_NULL_HANDLE : old_swap_chain->swap_chain;

	if (vkCreateSwapchainKHR(device.get_logical_device(), &create_info, nullptr, &swap_chain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain");
	}

	vkGetSwapchainImagesKHR(
		device.get_logical_device(),
		swap_chain,
		&image_count,
		nullptr
	);

	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(
		device.get_logical_device(),
		swap_chain,
		&image_count,
		swap_chain_images.data()
	);

	swap_chain_image_format = surface_format.format;
	swap_chain_extent = extent;
}

void game_engine::SwapChain::create_image_views()
{
	swap_chain_image_views.resize(swap_chain_images.size());
	for (size_t i = 0; i < swap_chain_images.size(); i++)
	{
		VkImageViewCreateInfo image_view_info{};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.image = swap_chain_images[i];
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = swap_chain_image_format;
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(
			device.get_logical_device(),
			&image_view_info,
			nullptr,
			&swap_chain_image_views[i]
		) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views");
		}
	}
}

void game_engine::SwapChain::create_render_pass()
{
	VkAttachmentDescription depth_attachment{};
	depth_attachment.format = find_depth_format();
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref{};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription color_attachment{};
	color_attachment.format = get_swap_chain_image_format();
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstSubpass = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_info.pAttachments = attachments.data();
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(
		device.get_logical_device(),
		&render_pass_info,
		nullptr,
		&render_pass
	) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}
}

void game_engine::SwapChain::create_depth_resources()
{
	VkFormat depth_format = find_depth_format();
	swap_chain_depth_format = depth_format;
	VkExtent2D swap_chain_extent = get_swap_chain_extent();

	depth_images.resize(image_count());
	depth_image_memories.resize(image_count());
	depth_image_views.resize(image_count());

	for (int i = 0; i < depth_images.size(); i++)
	{
		VkImageCreateInfo image_info{};

		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = swap_chain_extent.width;
		image_info.extent.height = swap_chain_extent.height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.format = depth_format;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.flags = 0;

		device.create_image_with_info(
			image_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depth_images[i],
			depth_image_memories[i]
		);

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = depth_images[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = depth_format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(
			device.get_logical_device(),
			&view_info,
			nullptr,
			&depth_image_views[i]
		) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create depth image view");
		}
	}
}

void game_engine::SwapChain::create_framebuffers()
{
	swap_chain_framebuffers.resize(image_count());

	for (size_t i = 0; i < image_count(); i++)
	{
		std::array<VkImageView, 2> attachments = { swap_chain_image_views[i], depth_image_views[i] };

		VkExtent2D swap_chain_extent = get_swap_chain_extent();
		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = swap_chain_extent.width;
		framebuffer_info.height = swap_chain_extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(
			device.get_logical_device(),
			&framebuffer_info,
			nullptr,
			&swap_chain_framebuffers[i]
		) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer");
		}
	}
}

void game_engine::SwapChain::create_sync_objects()
{
	image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	images_in_flight.resize(image_count(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(
			device.get_logical_device(),
			&semaphore_info,
			nullptr,
			&image_available_semaphores[i]
			) != VK_SUCCESS ||
			vkCreateSemaphore(
				device.get_logical_device(),
				&semaphore_info,
				nullptr,
				&render_finished_semaphores[i]
			) != VK_SUCCESS ||
			vkCreateFence(
				device.get_logical_device(),
				&fence_info,
				nullptr,
				&in_flight_fences[i]
			) != VK_SUCCESS
			)
		{
			throw std::runtime_error("Failed to create synchronization objects for a frame");
		}
	}
}

VkSurfaceFormatKHR game_engine::SwapChain::choose_swap_surface_format(
	const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	for (const auto& available_format : available_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			std::cout << "Surface format: B8G8R8A8 SRGB" << std::endl;
			return available_format;
		}
	}

	return available_formats[0];
}

VkPresentModeKHR game_engine::SwapChain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			std::cout << "Present mode: Mailbox" << std::endl;
			return available_present_mode;
		}
	}

	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			std::cout << "Present mode: Immediate" << std::endl;
			return available_present_mode;
		}
	}

	std::cout << "Present mode: FIFO" << std::endl;
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D game_engine::SwapChain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actual_extent = window_extent;
		actual_extent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actual_extent.width)
		);
		actual_extent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actual_extent.height)
		);
		return actual_extent;
	}
}

#include "renderer.h"

VkRenderPass game_engine::Renderer::get_swap_chain_render_pass() const
{
	return swap_chain->get_render_pass();
}

float game_engine::Renderer::get_aspect_ratio() const
{
	return swap_chain->extent_aspect_ratio();
}

bool game_engine::Renderer::is_frame_in_progress() const
{
	return is_frame_started;
}

VkCommandBuffer game_engine::Renderer::get_current_command_buffer() const
{
	assert(is_frame_started && "Cannot get command buffer when frame is not in progress");
	return command_buffers[current_frame_index];
}

int game_engine::Renderer::get_frame_index() const
{
	assert(is_frame_in_progress() && "Cannot get frame index when frame is not in progress");
	return current_frame_index;
}

VkCommandBuffer game_engine::Renderer::begin_frame()
{
	assert(!is_frame_in_progress() && "Cannot start new frame while another is still in progress");

	auto result = swap_chain->acquire_next_image(&current_image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate_swap_chain();
		return nullptr;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire next image");
	}

	is_frame_started = true;

	auto command_buffer = get_current_command_buffer();

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer");
	}

	return command_buffer;
}

void game_engine::Renderer::end_frame()
{
	assert(is_frame_in_progress() && "Cannot end frame when none is in progress");

	auto command_buffer = get_current_command_buffer();

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer");
	}

	auto result = swap_chain->submit_command_buffers(&command_buffer, &current_image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.was_window_resized())
	{
		window.reset_window_resized_flag();
		recreate_swap_chain();
		is_frame_started = false;
		return;
	}
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffers");
	}

	is_frame_started = false;
	current_frame_index = (current_frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void game_engine::Renderer::begin_swap_chain_render_pass(VkCommandBuffer command_buffer)
{
	assert(is_frame_in_progress() && "Cannot begin render pass when frame is not in progress");
	assert(command_buffer == get_current_command_buffer() && "Can only begin render pass for command buffer of current frame");

	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = swap_chain->get_render_pass();
	render_pass_info.framebuffer = swap_chain->get_framebuffer(current_image_index);

	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = swap_chain->get_swap_chain_extent();

	std::array<VkClearValue, 2> clear_values{};
	clear_values[0].color = { 0.005f, 0.005f, 0.005f, 1.0f };
	clear_values[1].depthStencil = { 1.0f, 0 };
	render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(
		command_buffer,
		&render_pass_info,
		VK_SUBPASS_CONTENTS_INLINE
	);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swap_chain->get_swap_chain_extent().width);
	viewport.height = static_cast<float>(swap_chain->get_swap_chain_extent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, swap_chain->get_swap_chain_extent() };
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void game_engine::Renderer::end_swap_chain_render_pass(VkCommandBuffer command_buffer)
{
	assert(is_frame_in_progress() && "Cannot end render pass when frame is not in progress");
	assert(command_buffer == get_current_command_buffer() && "Can only end render pass for command buffer of current frame");

	vkCmdEndRenderPass(command_buffer);
}

game_engine::Renderer::Renderer(Window& window, Device& device) : window(window), device(device)
{
	recreate_swap_chain();
	create_command_buffers();
}

game_engine::Renderer::~Renderer()
{
	free_command_buffers();
}

void game_engine::Renderer::create_command_buffers()
{
	command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocate_info{};

	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandPool = device.get_command_pool();
	allocate_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

	if (vkAllocateCommandBuffers(
		device.get_logical_device(),
		&allocate_info,
		command_buffers.data()
	) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}
}

void game_engine::Renderer::free_command_buffers()
{
	vkFreeCommandBuffers(
		device.get_logical_device(),
		device.get_command_pool(),
		static_cast<uint32_t>(command_buffers.size()),
		command_buffers.data()
	);

	command_buffers.clear();
}

void game_engine::Renderer::recreate_swap_chain()
{
	VkExtent2D extent = window.get_extent();
	while (extent.width == 0 || extent.height == 0)
	{
		extent = window.get_extent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device.get_logical_device());

	if (swap_chain == nullptr)
	{
		swap_chain = std::make_unique<SwapChain>(device, extent);
	}
	else
	{
		std::shared_ptr<SwapChain> old_swap_chain = std::move(swap_chain);
		swap_chain = std::make_unique<SwapChain>(device, extent, old_swap_chain);

		if (!old_swap_chain->compare_swap_formats(*swap_chain.get()))
		{
			throw std::runtime_error("Swap chain image format or color space has changed");
		}
	}
}
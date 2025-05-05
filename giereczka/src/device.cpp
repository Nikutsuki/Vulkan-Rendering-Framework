#include "device.h"

namespace game_engine {
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
    (
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT
    (
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEX
    (
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

	Device::Device(Window& window) : window(window)
	{
		create_instance();
		setup_debug_messenger();
		create_surface();
		pick_physical_device();
		create_logical_device();
		create_command_pool();
	}

    Device::~Device()
    {
		vkDestroyCommandPool(device_, command_pool, nullptr);
		vkDestroyDevice(device_, nullptr);

		if (enable_validation_layers)
		{
			DestroyDebugUtilsMessengerEX(instance, debug_messenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface_, nullptr);
		vkDestroyInstance(instance, nullptr);
    }

    VkCommandPool Device::get_command_pool()
    {
		return command_pool;
    }

    VkDevice Device::get_logical_device()
    {
		return device_;
    }

    VkSurfaceKHR Device::get_surface()
    {
        return surface_;
    }

    VkQueue Device::get_graphics_queue()
    {
        return graphics_queue;
    }

    VkQueue Device::get_present_queue()
    {
        return present_queue;
    }

    VkInstance Device::get_instance()
    {
		return instance;
    }

    SwapChainSupportDetails Device::get_swap_chain_support()
    {
		return query_swap_chain_support(physical_device);
    }

    uint32_t Device::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
				return i;
            }
        }

		throw std::runtime_error("failed to find suitable memory type");
    }

    QueueFamilyIndices Device::find_physical_queue_families()
    {
		return find_queue_families(physical_device);
    }

    VkFormat Device::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
				return format;
			}
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
				return format;
            }
        }

		throw std::runtime_error("failed to find supported format");
    }

    void Device::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device_, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create buffer");
        }

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(device_, buffer, &mem_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);
        if (vkAllocateMemory(device_, &alloc_info, nullptr, &bufferMemory) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to allocate buffer memory");
        }

		vkBindBufferMemory(device_, buffer, bufferMemory, 0);
    }

    VkCommandBuffer Device::begin_single_time_commands()
    {
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);
		return command_buffer;
    }

    void Device::end_single_time_commands(VkCommandBuffer command_buffer)
    {
		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue);

		vkFreeCommandBuffers(device_, command_pool, 1, &command_buffer);
    }

    void Device::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
    {
		VkCommandBuffer command_buffer = begin_single_time_commands();

		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;

		vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

		end_single_time_commands(command_buffer);
    }

    void Device::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count)
    {
		VkCommandBuffer command_buffer = begin_single_time_commands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layer_count;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(
            command_buffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
		end_single_time_commands(command_buffer);
    }

    void Device::create_image_with_info(const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory)
    {
        if (vkCreateImage(device_, &image_info, nullptr, &image) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create image");
        }

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(device_, image, &mem_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);
        if (vkAllocateMemory(device_, &alloc_info, nullptr, &image_memory) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to allocate image memory");
        }

        if (vkBindImageMemory(device_, image, image_memory, 0) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to bind image memory");
        }
    }

    void Device::create_instance()
    {
		if (enable_validation_layers && !check_validation_layer_support())
		{
			throw std::runtime_error("validation layers requested, but not available");
		}

		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Giereczka";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "No Engine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		auto extensions = get_required_extensions();
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (enable_validation_layers)
        {
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();

			populate_debug_messenger_create_info(debug_create_info);
			create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
		}
        else
        {
			create_info.enabledLayerCount = 0;
			create_info.pNext = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance");
        }

		has_glfw_required_instance_extensions();
    }

    void Device::setup_debug_messenger()
    {
		if (!enable_validation_layers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		populate_debug_messenger_create_info(create_info);
        if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to set up debug messenger");
        }
    }

    void Device::create_surface()
    {
		window.create_window_surface(instance, &surface_);
    }

    void Device::pick_physical_device()
    {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
        if (device_count == 0)
        {
			throw std::runtime_error("failed to find GPUs with Vulkan support");
        }

		std::cout << "Found " << device_count << " devices with Vulkan support" << std::endl;

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
        for (const auto& device : devices)
        {
            if (is_device_suitable(device))
            {
				physical_device = device;
				break;
            }
        }
    }

    void Device::create_logical_device()
    {
		QueueFamilyIndices indices = find_queue_families(physical_device);

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_families = { indices.graphics_family, indices.present_family };

		float queue_priority = 1.0f;
        for (uint32_t queue_family : unique_queue_families)
        {
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
        }

		VkPhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = VK_TRUE;

    	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features{};
    	indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    	indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
    	indexing_features.runtimeDescriptorArray = VK_TRUE;
    	indexing_features.descriptorBindingVariableDescriptorCount = VK_TRUE;
    	indexing_features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    	indexing_features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    	create_info.pNext = &indexing_features;

    	create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    	create_info.pQueueCreateInfos = queue_create_infos.data();
    	create_info.pEnabledFeatures = &device_features;
    	create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    	create_info.ppEnabledExtensionNames = device_extensions.data();

        if (enable_validation_layers)
        {
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		}
		else
		{
			create_info.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physical_device, &create_info, nullptr, &device_) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create logical device");
        }

		vkGetDeviceQueue(device_, indices.graphics_family, 0, &graphics_queue);
		vkGetDeviceQueue(device_, indices.present_family, 0, &present_queue);
    }

    void Device::create_command_pool()
    {
		QueueFamilyIndices queue_family_indices = find_queue_families(physical_device);

		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = queue_family_indices.graphics_family;
        pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create command pool");
        }
    }

    bool Device::is_device_suitable(VkPhysicalDevice device)
    {
		QueueFamilyIndices indices = find_queue_families(device);

		bool extensions_supported = check_device_extension_support(device);

		bool swap_chain_adequate = false;
        if (extensions_supported)
        {
			SwapChainSupportDetails swap_chain_support = query_swap_chain_support(device);
			swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
        }

		VkPhysicalDeviceFeatures supported_features;
		vkGetPhysicalDeviceFeatures(device, &supported_features);

    	bool descriptor_indexing_supported = check_descriptor_indexing_support(device);

    	return indices.is_complete() &&
			   extensions_supported &&
			   swap_chain_adequate &&
			   supported_features.samplerAnisotropy &&
			   descriptor_indexing_supported;
    }

    std::vector<const char*> Device::get_required_extensions()
    {
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions;

		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

        if (enable_validation_layers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

		return extensions;
    }

    bool Device::check_validation_layer_support()
    {
		uint32_t layer_count;

		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
        for (const char* layer_name : validation_layers)
        {
			bool layer_found = false;
            for (const auto& layer_properties : available_layers)
            {
                if (strcmp(layer_name, layer_properties.layerName) == 0)
                {
					layer_found = true;
					break;
                }
            }

            if (!layer_found)
            {
				return false;
            }
        }

		return true;
    }

    QueueFamilyIndices Device::find_queue_families(VkPhysicalDevice device)
    {
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queue_family_count,
            queue_families.data());

		int i = 0;
        for (const auto& queue_family : queue_families)
        {
            if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
				indices.graphics_family = i;
				indices.graphics_family_has_value = true;
            }

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);
            if (queue_family.queueCount > 0 && present_support)
            {
				indices.present_family = i;
				indices.present_family_has_value = true;
            }

            if (indices.is_complete())
            {
				break;
            }

			i++;
        }

		return indices;
    }

    void Device::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
    {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        create_info.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        create_info.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debugCallback;
        create_info.pUserData = nullptr;
    }

    void Device::has_glfw_required_instance_extensions()
    {
		uint32_t extension_count = 0;

		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

		std::cout << "available extensions:" << std::endl;
		std::unordered_set<std::string> available_extension_names;
        for (const auto& extension : available_extensions)
        {
			std::cout << '\t' << extension.extensionName << std::endl;
			available_extension_names.insert(extension.extensionName);
        }

		std::cout << "required extensions:" << std::endl;
		auto required_extensions = get_required_extensions();
        for (const auto& required_extension : required_extensions)
        {
			std::cout << '\t' << required_extension << std::endl;
            if (available_extension_names.find(required_extension) == available_extension_names.end())
            {
				throw std::runtime_error("missing required extension");
            }
        }
    }

    bool Device::check_device_extension_support(VkPhysicalDevice device)
    {
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &extension_count,
            available_extensions.data()
        );

		std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
        for (const auto& extension : available_extensions)
        {
			required_extensions.erase(extension.extensionName);
        }

		return required_extensions.empty();
    }

    SwapChainSupportDetails Device::query_swap_chain_support(VkPhysicalDevice device)
    {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            surface_,
            &format_count,
            nullptr
        );
        if (format_count != 0)
        {
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				device,
				surface_,
				&format_count,
				details.formats.data()
			);
        }

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device,
			surface_,
			&present_mode_count,
			nullptr
		);

        if (present_mode_count != 0)
        {
			details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface_,
                &present_mode_count,
                details.present_modes.data()
            );
        }

		return details;
    }
}

bool game_engine::Device::check_descriptor_indexing_support(VkPhysicalDevice device) {
	// Query descriptor indexing support
	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features{};
	indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;

	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.pNext = &indexing_features;

	vkGetPhysicalDeviceFeatures2(device, &features2);

	return indexing_features.descriptorBindingPartiallyBound &&
		   indexing_features.runtimeDescriptorArray &&
		   indexing_features.descriptorBindingVariableDescriptorCount;
}
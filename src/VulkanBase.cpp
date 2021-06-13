#include <VulkanBase.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	printf("Validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
};

#define DEBUG

void VulkanBase::init(SDL_Window* window) {

#ifdef DEBUG
	AddLayerProperty("VK_LAYER_KHRONOS_validation");
	AddInstanceExtension("VK_EXT_debug_utils");
	AddInstanceExtension("VK_EXT_debug_report");
#endif // DEBUG

	// Add surface extensions from SDL window
	uint32_t sdlExtensionsCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, nullptr);
	size_t additionalExtensionsSize = instanceExtensions.size();
	instanceExtensions.resize(instanceExtensions.size() + sdlExtensionsCount);
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, instanceExtensions.data() + additionalExtensionsSize);

	vk::ApplicationInfo appInfo = { "Vulkan Renderer", 1, "Vulkan Renderer", 1, VK_API_VERSION_1_1 };
	vk::InstanceCreateInfo instanceInfo(
		vk::InstanceCreateFlags(),
		&appInfo,
		layerProperties, 
		instanceExtensions
	);
	instance = vk::createInstance(instanceInfo);

#ifdef DEBUG
	// TODO: Find some additional information about vk::DispatchLoaderDynamic
	const auto dispatcher = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

	// TODO: Read some info about debug messengers in Vulkan
	auto debugMessenger = instance.createDebugUtilsMessengerEXT(
		vk::DebugUtilsMessengerCreateInfoEXT{
			{},
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			debugCallback
		},
		nullptr,
		dispatcher
	);
#endif

	for (const auto& physicalDevice : instance.enumeratePhysicalDevices()) {
		vk::PhysicalDeviceProperties gpuProps = physicalDevice.getProperties();
		gpu = strstr(gpuProps.deviceName, "NVIDIA") != nullptr ? physicalDevice : nullptr;
	}

	VkSurfaceKHR vksurface;
	SDL_bool res = SDL_Vulkan_CreateSurface(window, instance, &vksurface);
	surface = vksurface;

	std::vector<vk::QueueFamilyProperties> queueFamilyProps = gpu.getQueueFamilyProperties();
	const auto graphicProp = std::find_if(queueFamilyProps.begin(), queueFamilyProps.end(),
		[](vk::QueueFamilyProperties qfp) {
		return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
	});

	queues.graphicsQueueIndex = std::distance(queueFamilyProps.begin(), graphicProp);

	// TODO: We have to find different index if it equals to queueFamilyProps.size()
	queues.presentQueueIndex = gpu.getSurfaceSupportKHR(queues.graphicsQueueIndex, surface)
		? queues.graphicsQueueIndex
		: queueFamilyProps.size();

	float queuePriotiry = 1.0f;
	vk::DeviceQueueCreateInfo deviceQueueInfo(vk::DeviceQueueCreateFlags(), queues.graphicsQueueIndex, 1, &queuePriotiry);

	AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	device = gpu.createDevice(
		vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1u, &deviceQueueInfo, 
			0u, nullptr,
			static_cast<uint32_t>(deviceExtensions.size()),
			deviceExtensions.data()
		)
	);

	queues.graphicsQueue = device.getQueue(queues.graphicsQueueIndex, 0);

	formats = gpu.getSurfaceFormatsKHR(surface);

	// TODO: should check is there more than 1 formats and is it define
	format = formats.front().format; // is this color format???

	// TODO: read about surface capabilities
	surfaceCapabilities = gpu.getSurfaceCapabilitiesKHR(surface);

	// TODO: read about Extent2D
	swapchainExtent = surfaceCapabilities.currentExtent;

	// TODO: clarify present modes
	swapchainPresentMode = vk::PresentModeKHR::eFifo;

	// TODO: what is it?
	vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: surfaceCapabilities.currentTransform;

	vk::CompositeAlphaFlagBitsKHR compositeAlpha = (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
		: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
		: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit
		: vk::CompositeAlphaFlagBitsKHR::eOpaque;

	// TODO: find more informations about parts of this struct
	vk::SwapchainCreateInfoKHR swapchainInfo(
		vk::SwapchainCreateFlagsKHR(),
		surface,
		surfaceCapabilities.minImageCount,
		format,
		vk::ColorSpaceKHR::eSrgbNonlinear,
		swapchainExtent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		{},
		preTransform,
		compositeAlpha,
		swapchainPresentMode,
		true,
		nullptr
	);

	swapchain = device.createSwapchainKHR(swapchainInfo);

	swapchainImages = device.getSwapchainImagesKHR(swapchain);
	imageViews.reserve(swapchainImages.size());

	// TODO: read about componentMapping and ImageSubresourceRange
	vk::ComponentMapping componentMapping(
		vk::ComponentSwizzle::eR,
		vk::ComponentSwizzle::eG,
		vk::ComponentSwizzle::eB,
		vk::ComponentSwizzle::eA
	);

	vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	for (const auto& image : swapchainImages) {
		vk::ImageViewCreateInfo imageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			image,
			vk::ImageViewType::e2D,
			format,
			componentMapping,
			subResourceRange
		);
		imageViews.push_back(device.createImageView(imageViewCreateInfo));
	}
}

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vulkan/vulkan.hpp>

#include <stdio.h>
#include <vector>
#include <algorithm>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	printf("Validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
		"VulkanEngine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1024, 768,
		SDL_WINDOW_VULKAN
	);

	SDL_Event event;
	bool isShouldClose = false;

	std::vector<const char*> layerProperties{
		"VK_LAYER_KHRONOS_validation"
	};

	uint32_t sdlExtensionsCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, nullptr);
	std::vector<const char*> instanceExtensions = {
		// Extension VK_KHR_surface will get by SDL and I think it doesnt need it vector 
		//"VK_KHR_surface",

		// TODO: Find differences in utils and report
		"VK_EXT_debug_report",
		"VK_EXT_debug_utils"
	};
	size_t additionalExtensionsSize = instanceExtensions.size();
	instanceExtensions.resize(instanceExtensions.size() + sdlExtensionsCount);
	SDL_bool ress = SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, instanceExtensions.data() + additionalExtensionsSize);

	vk::ApplicationInfo appInfo = { "VulkanEngineApp", 1, "VulkanEngine", 1, VK_API_VERSION_1_1 };
	vk::InstanceCreateInfo instanceInfo = { {}, &appInfo };
	instanceInfo.enabledLayerCount = layerProperties.size();
	instanceInfo.ppEnabledLayerNames = layerProperties.data();
	instanceInfo.enabledExtensionCount = instanceExtensions.size();
	instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
	vk::Instance instance = vk::createInstance(instanceInfo);
	
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

	vk::PhysicalDevice gpu;
	for (const auto& physicalDevice : instance.enumeratePhysicalDevices()) {
		vk::PhysicalDeviceProperties gpuProps = physicalDevice.getProperties();
		gpu = strstr(gpuProps.deviceName, "NVIDIA") != nullptr ? physicalDevice : nullptr;
	}

	VkSurfaceKHR vksurface;
	SDL_bool res = SDL_Vulkan_CreateSurface(window, instance, &vksurface);
	vk::SurfaceKHR surface(vksurface);

	std::vector<vk::QueueFamilyProperties> queueFamilyProps = gpu.getQueueFamilyProperties();
	const auto graphicProp = std::find_if(queueFamilyProps.begin(), queueFamilyProps.end(),
		[](vk::QueueFamilyProperties qfp) {
		return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
	});
	size_t graphicsQueueFamilyIndex = std::distance(queueFamilyProps.begin(), graphicProp);

	// TODO: We have to find different index if it equals to queueFamilyProps.size()
	size_t presentQueueFamilyIndex = gpu.getSurfaceSupportKHR(static_cast<uint32_t>(graphicsQueueFamilyIndex), surface) 
		? graphicsQueueFamilyIndex
		: queueFamilyProps.size();

	float queuePriotiry = 1.0f;
	vk::DeviceQueueCreateInfo deviceQueueInfo(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(graphicsQueueFamilyIndex), 1, &queuePriotiry);

	std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk::Device device = gpu.createDevice(
		vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1u, &deviceQueueInfo, 
			0u, nullptr,
			static_cast<uint32_t>(deviceExtensions.size()),
			deviceExtensions.data()
		)
	);

	std::vector<vk::SurfaceFormatKHR> formats = gpu.getSurfaceFormatsKHR(surface);

	// TODO: should check is there more than 1 formats and is it define
	vk::Format format = formats.front().format;

	// TODO: read about surface capabilities
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = gpu.getSurfaceCapabilitiesKHR(surface);

	// TODO: read about Extent2D
	VkExtent2D swapchainExtent = surfaceCapabilities.currentExtent;

	// TODO: clarify present modes
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

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

	std::vector<uint32_t> queueFamilyIndexes = {
		static_cast<uint32_t>(graphicsQueueFamilyIndex),
		static_cast<uint32_t>(presentQueueFamilyIndex)
	};

	const auto iter = std::unique(queueFamilyIndexes.begin(), queueFamilyIndexes.end());
	queueFamilyIndexes.erase(queueFamilyIndexes.begin(), iter);

	// TODO: find more informations about parts of this struct
	vk::SwapchainCreateInfoKHR swapchainInfo(
		vk::SwapchainCreateFlagsKHR(),
		surface ,
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

	vk::SwapchainKHR swapchain = device.createSwapchainKHR(swapchainInfo);

	std::vector<vk::Image> swapchainImages = device.getSwapchainImagesKHR(swapchain);
	std::vector<vk::ImageView> imageViews;
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

	vk::CommandPool commandPool = device.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(), graphicsQueueFamilyIndex));

	vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1)
	).front();

	while (!isShouldClose) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
				isShouldClose = true;
		}
	}

	device.freeCommandBuffers(commandPool, commandBuffer);
	device.destroyCommandPool(commandPool);
	for (auto& imageView : imageViews) {
		device.destroyImageView(imageView);
	}
	device.destroySwapchainKHR(swapchain);
	instance.destroySurfaceKHR(surface);
	device.destroy();
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dispatcher);
	instance.destroy();
	SDL_DestroyWindow(window);

	return 0;
}
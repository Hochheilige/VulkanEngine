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

	std::vector<const char*> instanceExtensions = {
		"VK_KHR_surface",
		"VK_EXT_debug_report",
		"VK_EXT_debug_utils"
	};

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

	std::vector<vk::QueueFamilyProperties> queueFamilyProps = gpu.getQueueFamilyProperties();
	const auto graphicProp = std::find_if(queueFamilyProps.begin(), queueFamilyProps.end(),
		[](vk::QueueFamilyProperties qfp) {
		return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
	});
	size_t graphicsQueueFamilyIndex = std::distance(queueFamilyProps.begin(), graphicProp);

	float queuePriotiry = 1.0f;
	vk::DeviceQueueCreateInfo deviceQueueInfo(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(graphicsQueueFamilyIndex), 1, &queuePriotiry);
	vk::Device device = gpu.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueInfo));

	while (!isShouldClose) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
				isShouldClose = true;
		}
	}

	device.destroy();
	instance.destroy();
	SDL_DestroyWindow(window);

	return 0;
}

#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>

#include <VkBootstrap.h>

#include <iostream>

#define VK_CHECK(x)                                                     \
	do {                                                                \
		VkResult err = x;                                               \
		if (err) {                                                      \
			std::cout << "Detected Vulkan Error: " << err << std::endl; \
			abort();                                                    \
		}                                                               \
	} while (0)	                                                        \

VulkanEngine::VulkanEngine(uint32_t width, uint32_t height) : windowExtent({ width, height }) {
}

void VulkanEngine::init()
{
	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN);
	
	window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowExtent.width,
		windowExtent.height,
		window_flags
	);
	
	InitVulkan();
	InitSwapchain();

	//everything went fine
	isInitialized = true;
}

void VulkanEngine::cleanup()
{	
	if (isInitialized) {

		vkDestroySwapchainKHR(device, swapchain, nullptr);

		for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		}

		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkb::destroy_debug_utils_messenger(instance, debugMessenger);
		vkDestroyInstance(instance, nullptr);
		SDL_DestroyWindow(window);
	}
}

void VulkanEngine::draw()
{
	//nothing yet
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT) bQuit = true;
			if (e.type == SDL_KEYDOWN) std::cout << (char)e.key.keysym.sym << " Key pressed\n";
		}

		draw();
	}
}

void VulkanEngine::InitVulkan() {
	vkb::InstanceBuilder builder;

	// make vulkan instance with basic debug features
	auto instRet = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(true)
		.require_api_version(1, 1, 0) // maybe make sence leave it 1.1
		.use_default_debug_messenger()
		.build();

	vkb::Instance inst = instRet.value();

	instance = inst.instance;
	debugMessenger = inst.debug_messenger;

	SDL_Vulkan_CreateSurface(window, instance, &surface);

	vkb::PhysicalDeviceSelector selector{ inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(surface)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

	device = vkbDevice.device;
	gpu = physicalDevice.physical_device;
}

void VulkanEngine::InitSwapchain() {
	vkb::SwapchainBuilder builder{ gpu, device, surface };
	vkb::Swapchain vkbSwapchain = builder
		.use_default_format_selection()
		// use vSync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(windowExtent.width, windowExtent.height)
		.build()
		.value();

	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
	swapchainImageFormat = vkbSwapchain.image_format;
}


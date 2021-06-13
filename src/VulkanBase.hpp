#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vulkan/vulkan.hpp>

#include <vector>

class VulkanBase {
public:
	VulkanBase() = default;
	~VulkanBase() {}

	void init(SDL_Window* window);

	inline void AddLayerProperty(const char* property) { 
		layerProperties.push_back(property); 
	}

	inline void AddInstanceExtension(const char* extension) {
		instanceExtensions.push_back(extension);
	}

	inline void AddDeviceExtension(const char* extension) {
		deviceExtensions.push_back(extension);
	}



public:
	vk::Instance instance;
	vk::PhysicalDevice gpu;
	vk::Device device;
	vk::SurfaceKHR surface;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties;

	struct {
		vk::Queue graphicsQueue;
		uint32_t graphicsQueueIndex;
		vk::Queue presentQueue;
		uint32_t presentQueueIndex;
		vk::Queue computeQueue;
		uint32_t computeQueueIndex;
	} queues;

	std::vector<vk::SurfaceFormatKHR> formats;

	// TODO: should check is there more than 1 formats and is it define
	vk::Format format; // is this color format???

	// TODO: read about surface capabilities
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;

	// TODO: read about Extent2D
	vk::Extent2D swapchainExtent;

	// TODO: clarify present modes
	vk::PresentModeKHR swapchainPresentMode;

	vk::SwapchainKHR swapchain;

	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> imageViews;

	std::vector<const char*> layerProperties;
	std::vector<const char*> instanceExtensions;
	std::vector<const char*> deviceExtensions;
};
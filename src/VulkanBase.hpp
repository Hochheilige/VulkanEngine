#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vulkan/vulkan.hpp>

#include <vector>

struct Queues {
	vk::Queue graphicsQueue;
	uint32_t graphicsQueueIndex;
	vk::Queue presentQueue;
	uint32_t presentQueueIndex;
	vk::Queue computeQueue;
	uint32_t computeQueueIndex;
};

class VulkanBase {
public:
	VulkanBase() = default;
	~VulkanBase();

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

	const vk::Instance& GetInstance() { return instance; }
	const vk::PhysicalDevice& GetPhysicalDevice() { return gpu; }
	const vk::Device& GetDevice() { return device; }
	const vk::SurfaceKHR& GetSurface() { return surface; }
	const Queues& GetQueues() { return queues; }
	const vk::Format& GetFormat() { return format; }
	const vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities() { return surfaceCapabilities; }


private:
	vk::Instance instance;
	vk::PhysicalDevice gpu;
	vk::Device device;
	vk::SurfaceKHR surface;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties;

	Queues queues;

	std::vector<vk::SurfaceFormatKHR> formats;

	// TODO: should check is there more than 1 formats and is it define
	vk::Format format; // is this color format???

	// TODO: read about surface capabilities
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;

#ifndef NDEBUD
	vk::DispatchLoaderDynamic dispatcher;
	vk::DebugUtilsMessengerEXT messenger;
#endif // !NDEBUD


	std::vector<const char*> layerProperties;
	std::vector<const char*> instanceExtensions;
	std::vector<const char*> deviceExtensions;
};
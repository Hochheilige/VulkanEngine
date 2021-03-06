#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vulkan/vulkan.hpp>

#include <vector>

struct Queues {
	vk::Queue graphicsQueue;
	uint32_t graphicsQueueIndex = 0;
	vk::Queue presentQueue;
	uint32_t presentQueueIndex = 0;
	vk::Queue computeQueue;
	uint32_t computeQueueIndex = 0;
};

class VulkanBase {
public:
	VulkanBase() {};
	~VulkanBase();

	void Init(SDL_Window* window);

	inline void AddLayerProperty(const char* property) { 
		layerProperties.push_back(property); 
	}

	inline void AddInstanceExtension(const char* extension) {
		instanceExtensions.push_back(extension);
	}

	inline void AddDeviceExtension(const char* extension) {
		deviceExtensions.push_back(extension);
	}

	const vk::Instance& GetInstance() const { return instance; }
	const vk::PhysicalDevice& GetPhysicalDevice() const { return gpu; }
	const vk::Device& GetDevice() const { return device; }
	const vk::SurfaceKHR& GetSurface() const { return surface; }
	const Queues& GetQueues() const { return queues; }
	const vk::Format& GetFormat() const { return format; }
	const vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities() const { return surfaceCapabilities; }


private:
	vk::Instance instance;
	vk::PhysicalDevice gpu;
	vk::Device device;
	vk::SurfaceKHR surface;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties;

	Queues queues;

	std::vector<vk::SurfaceFormatKHR> formats;

	// TODO: should check is there more than 1 formats and is it define
	vk::Format format{}; // is this color format???

	// TODO: read about surface capabilities
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;

#ifndef NDEBUD
	vk::DispatchLoaderDynamic dispatcher;
	vk::DebugUtilsMessengerEXT messenger;
#endif // !NDEBUD

	std::vector<const char*> layerProperties{};
	std::vector<const char*> instanceExtensions{};
	std::vector<const char*> deviceExtensions{};
};
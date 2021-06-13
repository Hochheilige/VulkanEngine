#pragma once

#include <vulkan/vulkan.hpp>

class Resource {
public:
	Resource(const vk::PhysicalDevice& gpu);
	virtual ~Resource() {}

	vk::PhysicalDeviceMemoryProperties& GetMemoryProperties() { return memoryProperties; }

protected:
	vk::PhysicalDeviceMemoryProperties memoryProperties;
};
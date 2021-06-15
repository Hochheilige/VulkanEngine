#pragma once

#include <vulkan/vulkan.hpp>

class Resource {
public:
	Resource() {}
	Resource(const vk::PhysicalDevice& gpu);
	virtual ~Resource() {}

	void SetPhysicalDeviceMemoryProperties(const vk::PhysicalDevice& gpu);

	vk::PhysicalDeviceMemoryProperties& GetMemoryProperties() { return memoryProperties; }

protected:
	vk::PhysicalDeviceMemoryProperties memoryProperties;
};
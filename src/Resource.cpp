#include "Resource.hpp"

Resource::Resource(const vk::PhysicalDevice& gpu) {
	memoryProperties = gpu.getMemoryProperties();
}

void Resource::SetPhysicalDeviceMemoryProperties(const vk::PhysicalDevice& gpu) {
	memoryProperties = gpu.getMemoryProperties();
}

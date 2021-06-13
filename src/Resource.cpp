#include "Resource.hpp"

Resource::Resource(const vk::PhysicalDevice& gpu) {
	memoryProperties = gpu.getMemoryProperties();
}

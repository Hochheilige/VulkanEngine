#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>

namespace utils {
	uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);


}
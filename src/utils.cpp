#include <utils.hpp>

namespace utils {

	uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask) {
		uint32_t typeIndex = uint32_t(~0);
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) &&
				((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask))
			{
				typeIndex = i;
				break;
			}
			typeBits >>= 1;
		}
		assert(typeIndex != uint32_t(~0));
		return typeIndex;
	}

}
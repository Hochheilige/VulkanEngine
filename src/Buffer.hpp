#pragma once

#include <vulkan/vulkan.hpp>

#include <Resource.hpp>
#include <VulkanBase.hpp>

// Можно сделать шаблонным не весь класс а только некоторые функции

class Buffer : public Resource {
public:
	Buffer(const vk::PhysicalDevice gpu, vk::BufferUsageFlagBits flags) 
		: Resource(gpu), flags(flags) {
	}
	~Buffer() {}

	template <class Object>
	void Init(const vk::Device& device) {
		buffer = device.createBuffer(
			vk::BufferCreateInfo(
				vk::BufferCreateFlags(),
				sizeof(Object),
				flags
			)
		);

		bufferMemoryRequiremenents = device.getBufferMemoryRequirements(buffer);
		uint32_t memoryTypeIndex = utils::findMemoryType(memoryProperties, bufferMemoryRequiremenents.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		deviceMemory = device.allocateMemory(vk::MemoryAllocateInfo(bufferMemoryRequiremenents.size, memoryTypeIndex));
	}

	template <class Object>
	void CopyBuffer(const vk::Device& device, const Object& object) {
		data = static_cast<uint8_t*>(device.mapMemory(deviceMemory, 0, bufferMemoryRequiremenents.size));
		memcpy(data, &object, sizeof(object));
		device.unmapMemory(deviceMemory);
		device.bindBufferMemory(buffer, deviceMemory, 0);
	}

	void MapBuffer(const vk::Device& device) {
		data = static_cast<uint8_t*>(device.mapMemory(deviceMemory, 0, bufferMemoryRequiremenents.size));
	}

	void UnMapBuffer(const vk::Device& device) {
		device.unmapMemory(deviceMemory);
	}

	void BindBuffer(const vk::Device& device) {
		device.bindBufferMemory(buffer, deviceMemory, 0);
	}

	template <class Object>
	void MemoryCopy(const Object& object) {
		memcpy(data, &object, sizeof(object));
	}

	const vk::Buffer& GetBuffer() { return buffer; }
	const vk::BufferView& GetBufferView() { return bufferView; }
	const vk::MemoryRequirements& GetMemoryRequirements() { return bufferMemoryRequiremenents; }
	const vk::DeviceMemory& GetDeviceMemory() { return deviceMemory; }
	const vk::BufferUsageFlagBits& GetBufferUsageFlags() { return flags; }
	const uint8_t* GetData() { return data; }

private:
	vk::Buffer buffer;
	vk::BufferView bufferView;
	vk::MemoryRequirements bufferMemoryRequiremenents;
	vk::DeviceMemory deviceMemory;
	vk::BufferUsageFlagBits flags;

	uint8_t* data = nullptr;
};
#include <Buffer.hpp>

void Buffer::CopyBuffer(const vk::Device& device, const std::vector<Vertex>& vertices) {
	data = static_cast<uint8_t*>(device.mapMemory(deviceMemory, 0, bufferMemoryRequiremenents.size));
	memcpy(data, vertices.data(), sizeof(Vertex) * vertices.size());
	device.unmapMemory(deviceMemory);
	device.bindBufferMemory(buffer, deviceMemory, 0);
}

void Buffer::InitResource(const vk::PhysicalDevice& gpu) {
	memoryProperties = gpu.getMemoryProperties();
}
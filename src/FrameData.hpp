#pragma once

#include <vulkan/vulkan.hpp>

#include <Buffer.hpp>

struct FrameData {
	vk::Semaphore presentSemaphore, renderSemaphore;
	vk::Fence renderFence;

	vk::CommandPool commandPool;
	vk::CommandBuffer commandBuffer;

	Buffer cameraBuffer;
	vk::DescriptorSet globalDescriptor;

	Buffer objectBuffer;
	vk::DescriptorSet objectDescriptor;
};
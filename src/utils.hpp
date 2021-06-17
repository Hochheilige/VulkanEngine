#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <fstream>

namespace utils {
	uint32_t findMemoryType(const vk::PhysicalDeviceMemoryProperties& memoryProperties, const uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);

	vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage, const vk::ShaderModule& shaderModule);

	vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo();

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo(vk::PrimitiveTopology topology);

	vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(vk::PolygonMode polygonMode);

	vk::PipelineMultisampleStateCreateInfo multisamplingStateCreateInfo();

	vk::PipelineColorBlendAttachmentState colorBlendAttachmentState();

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo();

	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo(bool isDepthTest, bool isDepthWrite, vk::CompareOp compareOp);

	vk::DescriptorSetLayoutBinding descriptorsetLayoutBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, const uint32_t binding);

	vk::WriteDescriptorSet writeDescriptorSet(vk::DescriptorType type, const vk::DescriptorSet& dstSet,
		const vk::DescriptorBufferInfo& bufferInfo, uint32_t binding);

	vk::ShaderModule loadShaderModule(const char* filePath, const vk::Device& device);
}
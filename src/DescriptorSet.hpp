#pragma once

#include <vulkan/vulkan.hpp>

class DescriptorSet {
public:
	DescriptorSet(uint32_t bindings, vk::DescriptorType descriptorType, 
		uint32_t descriptorCount, vk::ShaderStageFlagBits shaderStageFlag);
	~DescriptorSet();

	void CreateDescriptorSetAndPipelineLayouts(const vk::Device& device);

	void CreateDescriptorPool(const vk::Device& device, vk::DescriptorPoolCreateFlagBits flags, uint32_t maxSets);

	void CreateDescriptorSet(const vk::Device& device);

	template <typename Object>
	void UpdateDescriptorSet(const vk::Device& device, const vk::Buffer& buffer, const uint32_t offset = 0) {
		// descriptor buffer (what a....)
		vk::DescriptorBufferInfo descriptorBufferInfo(
			buffer,
			offset,
			sizeof(Object)
		);

		writeDescriptorSet = {
			descriptorSet,
			0,
			0,
			descriptorType,
			{},
			descriptorBufferInfo
		};
		device.updateDescriptorSets(writeDescriptorSet, nullptr);
	}

	const vk::DescriptorSetLayoutBinding& GetDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; }
	const vk::DescriptorSetLayout& GetDescriptorSetLayout() { return descriptorSetLayout; }
	const vk::DescriptorType& GetDescriptorType() { return descriptorType; }
	const vk::PipelineLayout& GetPipelineLayout() { return pipelineLayout; }
	const vk::DescriptorPoolSize& GetPoolSize() { return poolSize; }
	const vk::DescriptorPool& GetDescriptorPool() { return descriptorPool; }
	const vk::DescriptorSet& GetDescriptorSet() { return descriptorSet; }
	const vk::WriteDescriptorSet& GetWriteDescriptorSet() { return writeDescriptorSet; }

private:
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DescriptorType descriptorType;
	vk::PipelineLayout pipelineLayout;
	vk::DescriptorPoolSize poolSize;
	vk::DescriptorPool descriptorPool;
	vk::DescriptorSet descriptorSet;
	vk::WriteDescriptorSet writeDescriptorSet;
};
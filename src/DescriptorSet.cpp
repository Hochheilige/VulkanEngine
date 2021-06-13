#include "DescriptorSet.hpp"

DescriptorSet::DescriptorSet(uint32_t bindings, vk::DescriptorType descriptorType,
	uint32_t descriptorCount, vk::ShaderStageFlagBits shaderStageFlag)
	: descriptorType(descriptorType) {
	// TODO: make sure what is descriptor set need for
	// Descriptor set stuff and pipelineLayout
	descriptorSetLayoutBinding = {
		bindings,
		descriptorType,
		descriptorCount,
		shaderStageFlag
	};
}

DescriptorSet::~DescriptorSet() {

}

void DescriptorSet::CreateDescriptorSetAndPipelineLayouts(const vk::Device& device) {
	descriptorSetLayout = device.createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), descriptorSetLayoutBinding)
	);

	pipelineLayout = device.createPipelineLayout(
		vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout)
	);
}

void DescriptorSet::CreateDescriptorPool(const vk::Device& device, vk::DescriptorPoolCreateFlagBits flags, uint32_t maxSets) {
	// Descriptor pool loool
	poolSize = { descriptorType, 1 };
	descriptorPool = device.createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			flags,
			maxSets,
			poolSize
		)
	);
}

void DescriptorSet::CreateDescriptorSet(const vk::Device& device) {
	// allocate a desriptor set
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, descriptorSetLayout);
	descriptorSet = device.allocateDescriptorSets(descriptorSetAllocateInfo).front();
}


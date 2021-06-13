#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class PipelineBuilder {
public:
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	vk::Viewport viewport;
	vk::Rect2D scissor;
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	vk::PipelineColorBlendAttachmentState colorBlendAttachment;
	vk::PipelineMultisampleStateCreateInfo multisampling;
	vk::PipelineLayout pipelineLayout;
	vk::PipelineDepthStencilStateCreateInfo depthStencil;

	vk::Pipeline Build(const vk::Device& device, const vk::RenderPass& renderPass);
};
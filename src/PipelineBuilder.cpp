#include <PipelineBuilder.hpp>

vk::Pipeline PipelineBuilder::Build(const vk::Device& device, const vk::RenderPass& renderPass) {
	vk::PipelineViewportStateCreateInfo viewportState(
		vk::PipelineViewportStateCreateFlags(),
		1,
		&viewport,
		1,
		&scissor
	);

	vk::PipelineColorBlendStateCreateInfo colorBlending(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE,
		vk::LogicOp::eCopy,
		colorBlendAttachment
	);

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		vk::PipelineCreateFlags(),
		shaderStages,
		&vertexInputInfo,
		&inputAssembly,
		{},
		&viewportState,
		&rasterizer,
		&multisampling,
		& depthStencil,
		&colorBlending,
		{},
		pipelineLayout,
		renderPass
	);

	return device.createGraphicsPipeline(nullptr, pipelineInfo);
}
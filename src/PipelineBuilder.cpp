#include <PipelineBuilder.hpp>

vk::Pipeline PipelineBuilder::Build(const vk::Device& device, const vk::RenderPass& renderPass) {
	vk::PipelineViewportStateCreateInfo viewportState(
		vk::PipelineViewportStateCreateFlags(),
		1,
		nullptr,
		1,
		nullptr
	);

	vk::PipelineColorBlendStateCreateInfo colorBlending(
		vk::PipelineColorBlendStateCreateFlags(),
		false,
		vk::LogicOp::eNoOp,
		colorBlendAttachment,
		{ { 1.0f, 1.0f, 1.0f, 1.0f } }
	);

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		vk::PipelineCreateFlags(),
		shaderStages,
		&vertexInputInfo,
		&inputAssembly,
		nullptr,
		&viewportState,
		&rasterizer,
		&multisampling,
		&depthStencil,
		&colorBlending,
		&dynamicStateCreateInfo,
		pipelineLayout,
		renderPass
	);

	return device.createGraphicsPipeline(nullptr, pipelineInfo);
}
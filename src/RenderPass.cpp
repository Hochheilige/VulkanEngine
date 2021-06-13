#include "RenderPass.hpp"

RenderPass::RenderPass(const vk::Format& format, const vk::Format& depthFormat) {
	attachmentDescriptions[0] = vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(),
		format,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR
	);
	attachmentDescriptions[1] = vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(),
		depthFormat,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	colorReference = { 0, vk::ImageLayout::eColorAttachmentOptimal };
	depthReference = { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

	subpass = {
		vk::SubpassDescriptionFlags(),
		vk::PipelineBindPoint::eGraphics,
		{},
		colorReference,
		{},
		&depthReference
	};
}

RenderPass::~RenderPass()
{
}

void RenderPass::Create(const vk::Device& device) {
	renderPass = device.createRenderPass(
		vk::RenderPassCreateInfo(
			vk::RenderPassCreateFlags(),
			attachmentDescriptions,
			subpass
		)
	);
}

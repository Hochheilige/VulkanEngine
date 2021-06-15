#include "Framebuffer.hpp"

Framebuffer::Framebuffer() {
	frameBufferCreateInfo = {
		vk::FramebufferCreateFlags(),
		{},
		attachments, {}, {},
		1
	};
}

Framebuffer::Framebuffer(const vk::RenderPass& renderPass, const vk::Extent2D& extent,
	const vk::ImageView& depthImageView) {
	attachments[1] = depthImageView;

	frameBufferCreateInfo = {
		vk::FramebufferCreateFlags(),
		renderPass,
		attachments,
		extent.width,
		extent.height,
		1
	};
}

Framebuffer::~Framebuffer() {
}

void Framebuffer::Create(const vk::Device& device, const vk::RenderPass& renderPass, const Swapchain& swapchain,
	const vk::ImageView& depthImage) {
	frameBufferCreateInfo.setRenderPass(renderPass);
	frameBufferCreateInfo.setWidth(swapchain.GetExtent().width);
	frameBufferCreateInfo.setHeight(swapchain.GetExtent().height);
	framebuffers.reserve(swapchain.GetImageViews().size());
	attachments[1] = depthImage;
	for (const auto& imageView : swapchain.GetImageViews()) {
		attachments[0] = imageView;
		framebuffers.push_back(device.createFramebuffer(frameBufferCreateInfo));
	}
}

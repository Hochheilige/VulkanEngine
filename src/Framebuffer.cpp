#include "Framebuffer.hpp"

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

void Framebuffer::Create(const vk::Device& device, const Swapchain& swapchain) {
	framebuffers.reserve(swapchain.GetImageViews().size());
	for (const auto& imageView : swapchain.GetImageViews()) {
		attachments[0] = imageView;
		framebuffers.push_back(device.createFramebuffer(frameBufferCreateInfo));
	}
}

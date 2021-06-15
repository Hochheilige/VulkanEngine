#pragma once

#include <vulkan/vulkan.hpp>

#include <array>

class RenderPass {
public:
	RenderPass();
	~RenderPass();


	void Create(const vk::Device& device, const vk::Format& format, const vk::Format& depthFormat);

	const vk::AttachmentReference& GetColorReference() { return colorReference; }
	const vk::AttachmentReference& GetDepthReference() { return depthReference; }
	const vk::SubpassDescription& GetSubpass() { return subpass; }
	const vk::RenderPass& GetRenderPass() { return renderPass; }

private:
	std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
	vk::AttachmentReference colorReference;
	vk::AttachmentReference depthReference;
	vk::SubpassDescription subpass;
	vk::RenderPass renderPass;
};
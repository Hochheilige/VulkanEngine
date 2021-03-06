#pragma once

#include <vulkan/vulkan.hpp>

#include <array>

#include <Swapchain.hpp>

class Framebuffer {
public:
	Framebuffer();
	Framebuffer(const vk::RenderPass& renderPass, const vk::Extent2D& extent, const vk::ImageView& depthImageView);
	~Framebuffer();

	void Create(const vk::Device& device, const vk::RenderPass& renderPass, const Swapchain& swapchain, const vk::ImageView& depthImage);

	const std::array<vk::ImageView, 2>& GetAttchments() { return attachments; }
	const std::vector<vk::Framebuffer>& GetFramebuffers() { return framebuffers; }

private:
	std::array<vk::ImageView, 2> attachments;
	vk::FramebufferCreateInfo frameBufferCreateInfo;
	std::vector<vk::Framebuffer> framebuffers;
};
#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

#include <VulkanBase.hpp>

class Swapchain {
public:
	Swapchain() {}
	Swapchain(const VulkanBase& base);
	~Swapchain() {}

	void RecreateSwapchain(vk::Extent2D extent);

	void Init(const VulkanBase& base);

	const vk::SwapchainKHR& GetSwapchain() const { return swapchain; }
	const std::vector<vk::Image> GetImages() const { return images; }
	const std::vector<vk::ImageView> GetImageViews() const { return imageViews; }
	const vk::Extent2D& GetExtent() const { return extent; }
	const vk::PresentModeKHR GetPresentMode() const { return presentMode; }

private:
	vk::SwapchainKHR swapchain;
	std::vector<vk::Image> images;
	std::vector<vk::ImageView> imageViews;

	// TODO: read about Extent2D
	vk::Extent2D extent;
	// TODO: clarify present modes
	const vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
};
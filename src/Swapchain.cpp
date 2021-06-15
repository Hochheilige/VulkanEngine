#include "Swapchain.hpp"

Swapchain::Swapchain(const VulkanBase& base) {
	extent = base.GetSurfaceCapabilities().currentExtent;

	// TODO: what is it?
	vk::SurfaceTransformFlagBitsKHR preTransform = (base.GetSurfaceCapabilities().supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: base.GetSurfaceCapabilities().currentTransform;

	vk::CompositeAlphaFlagBitsKHR compositeAlpha = (base.GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
		: (base.GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
		: (base.GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit
		: vk::CompositeAlphaFlagBitsKHR::eOpaque;

	// TODO: find more informations about parts of this struct
	vk::SwapchainCreateInfoKHR swapchainInfo(
		vk::SwapchainCreateFlagsKHR(),
		base.GetSurface(),
		base.GetSurfaceCapabilities().minImageCount,
		base.GetFormat(),
		vk::ColorSpaceKHR::eSrgbNonlinear,
		extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		{},
		preTransform,
		compositeAlpha,
		presentMode,
		true,
		nullptr
	);

	swapchain = base.GetDevice().createSwapchainKHR(swapchainInfo);

	images = base.GetDevice().getSwapchainImagesKHR(swapchain);
	imageViews;
	imageViews.reserve(images.size());

	// TODO: read about componentMapping and ImageSubresourceRange
	vk::ComponentMapping componentMapping(
		vk::ComponentSwizzle::eR,
		vk::ComponentSwizzle::eG,
		vk::ComponentSwizzle::eB,
		vk::ComponentSwizzle::eA
	);

	vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	for (const auto& image : images) {
		vk::ImageViewCreateInfo imageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			image,
			vk::ImageViewType::e2D,
			base.GetFormat(),
			componentMapping,
			subResourceRange
		);
		imageViews.push_back(base.GetDevice().createImageView(imageViewCreateInfo));
	}
}

void Swapchain::RecreateSwapchain(vk::Extent2D extent)
{
}

void Swapchain::Init(const VulkanBase& base) {
	extent = base.GetSurfaceCapabilities().currentExtent;

	// TODO: what is it?
	vk::SurfaceTransformFlagBitsKHR preTransform = (base.GetSurfaceCapabilities().supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: base.GetSurfaceCapabilities().currentTransform;

	vk::CompositeAlphaFlagBitsKHR compositeAlpha = (base.GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
		: (base.GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
		: (base.GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit
		: vk::CompositeAlphaFlagBitsKHR::eOpaque;

	// TODO: find more informations about parts of this struct
	vk::SwapchainCreateInfoKHR swapchainInfo(
		vk::SwapchainCreateFlagsKHR(),
		base.GetSurface(),
		base.GetSurfaceCapabilities().minImageCount,
		base.GetFormat(),
		vk::ColorSpaceKHR::eSrgbNonlinear,
		extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		{},
		preTransform,
		compositeAlpha,
		presentMode,
		true,
		nullptr
	);

	swapchain = base.GetDevice().createSwapchainKHR(swapchainInfo);

	images = base.GetDevice().getSwapchainImagesKHR(swapchain);
	imageViews;
	imageViews.reserve(images.size());

	// TODO: read about componentMapping and ImageSubresourceRange
	vk::ComponentMapping componentMapping(
		vk::ComponentSwizzle::eR,
		vk::ComponentSwizzle::eG,
		vk::ComponentSwizzle::eB,
		vk::ComponentSwizzle::eA
	);

	vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	for (const auto& image : images) {
		vk::ImageViewCreateInfo imageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			image,
			vk::ImageViewType::e2D,
			base.GetFormat(),
			componentMapping,
			subResourceRange
		);
		imageViews.push_back(base.GetDevice().createImageView(imageViewCreateInfo));
	}
}

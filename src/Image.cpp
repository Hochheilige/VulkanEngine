#include "Image.hpp"

Image::Image(const vk::PhysicalDevice gpu, vk::Format format)
: Resource(gpu), format(format) {
	formatProperties = gpu.getFormatProperties(format);
}

Image::~Image() {
}

void Image::Init(const VulkanBase& base, vk::ImageAspectFlagBits flags) {

	// This is need only for depth image and this should be clear
	// TODO: tiling??????
	vk::ImageTiling tiling;
	if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
		tiling = vk::ImageTiling::eLinear;
	}
	else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
		tiling = vk::ImageTiling::eOptimal;
	}
	else {
		throw std::runtime_error("DepthStencilAttachment is not supported for D16Unorm depth format.");
	}

	vk::ImageCreateInfo imageInfo(
		vk::ImageCreateFlags(),
		vk::ImageType::e2D,
		format,
		vk::Extent3D(base.GetSurfaceCapabilities().currentExtent, 1),
		1, 1,
		vk::SampleCountFlagBits::e1,
		tiling,
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	);

	image = base.GetDevice().createImage(imageInfo);

	// TODO: Dealing with memory allocation in Vulkan
	vk::MemoryRequirements memoryRequirements = base.GetDevice().getImageMemoryRequirements(image);
	uint32_t typeBits = memoryRequirements.memoryTypeBits;
	uint32_t typeIndex = utils::findMemoryType(memoryProperties, typeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	// This part probably clear
	deviceMemory = base.GetDevice().allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));
	base.GetDevice().bindImageMemory(image, deviceMemory, 0);

	// TODO: what is component mapping and subresource range (2)
	vk::ImageSubresourceRange subResourceRange(flags, 0, 1, 0, 1);

	// TODO: read about componentMapping and ImageSubresourceRange
	vk::ComponentMapping componentMapping(
		vk::ComponentSwizzle::eR,
		vk::ComponentSwizzle::eG,
		vk::ComponentSwizzle::eB,
		vk::ComponentSwizzle::eA
	);

	imageView = base.GetDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			image,
			vk::ImageViewType::e2D,
			format,
			componentMapping,
			subResourceRange
		)
	);
}

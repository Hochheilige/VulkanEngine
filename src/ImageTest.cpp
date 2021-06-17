#include "Image.hpp"

Image::Image(const vk::PhysicalDevice gpu, vk::ImageUsageFlagBits fl, vk::Format format)
	: Resource(gpu), format(format), flags(fl) {
	formatProperties = gpu.getFormatProperties(format);
}

Image::Image(const vk::Extent3D ex) : extent(ex) {
}

Image::~Image() {
}

void Image::InitResource(const vk::PhysicalDevice& gpu) {
	memoryProperties = gpu.getMemoryProperties();
}

void Image::Init(const VulkanBase& base, vk::Format format, vk::ImageAspectFlagBits fl, bool isDepth) {
	if (extent.height == 0 && extent.width == 0) {
		extent = vk::Extent3D(base.GetSurfaceCapabilities().currentExtent, 1);
	}

	vk::ImageCreateInfo imageInfo(
		vk::ImageCreateFlags(),
		vk::ImageType::e2D,
		format,
		extent,
		1, 1,
		vk::SampleCountFlagBits::e1,
		{},
		{}
	);

	if (memoryProperties != base.GetPhysicalDevice().getMemoryProperties()) {
		InitResource(base.GetPhysicalDevice());
		SetFormat(base.GetPhysicalDevice(), format);
	}

	vk::ImageTiling tiling;
	vk::ImageSubresourceRange subResourceRange;
	if (isDepth) {
		// This is need only for depth image and this should be clear
		// TODO: tiling??????

		if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
			tiling = vk::ImageTiling::eLinear;
		}
		else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
			tiling = vk::ImageTiling::eOptimal;
		}
		else {
			throw std::runtime_error("DepthStencilAttachment is not supported for D16Unorm depth format.");
		}

		imageInfo.setTiling(tiling);
		imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
		subResourceRange = { fl, 0, 1, 0, 1 };
	}
	else {
		imageInfo.setUsage(flags);
		subResourceRange = { vk::ImageAspectFlags(), 0, 1, 0, 1 };
	}

	image = base.GetDevice().createImage(imageInfo);

	// TODO: Dealing with memory allocation in Vulkan
	vk::MemoryRequirements memoryRequirements = base.GetDevice().getImageMemoryRequirements(image);
	uint32_t typeBits = memoryRequirements.memoryTypeBits;
	uint32_t typeIndex = utils::findMemoryType(memoryProperties, typeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	// This part probably clear
	deviceMemory = base.GetDevice().allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));
	base.GetDevice().bindImageMemory(image, deviceMemory, 0);

	// TODO: what is component mapping and subresource range (2)


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

void Image::SetFormat(const vk::PhysicalDevice& gpu, vk::Format f) {
	format = f;
	formatProperties = gpu.getFormatProperties(format);
}

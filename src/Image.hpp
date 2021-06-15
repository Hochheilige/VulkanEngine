#pragma once

#include <vulkan/vulkan.hpp>

#include <VulkanBase.hpp>
#include <Resource.hpp>
#include <utils.hpp>

class Image : public Resource {
public:
	Image() {}
	Image(const vk::PhysicalDevice gpu, vk::Format format = vk::Format::eUndefined);
	~Image();

	void Init(const VulkanBase& base, vk::Format format, vk::ImageAspectFlagBits flags = vk::ImageAspectFlagBits::eColor, bool isDepth = false);

	const vk::Image& GetImage() { return image; }
	const vk::ImageView& GetImageView() { return imageView; }
	const vk::Format& GetFormat() { return format; }
	const vk::FormatProperties& GetFormatProperties() { return formatProperties; }
	const vk::DeviceMemory& GetDeviceMemory() { return deviceMemory; }

	void SetFormat(const vk::PhysicalDevice& gpu, vk::Format f);

private:
	vk::Image image;
	vk::ImageView imageView;
	vk::Format format;
	vk::FormatProperties formatProperties;
	vk::DeviceMemory deviceMemory;

	void InitResource(const vk::PhysicalDevice& gpu);
};
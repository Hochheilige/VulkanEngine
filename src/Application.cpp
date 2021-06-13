#include <SDL.h>
#include <SDL_vulkan.h>

#include <vulkan/vulkan.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <memory>

#include <Engine.hpp>

// Extra stuff to render a cube (hardcoded vertexes)
struct VertexPC
{
	float x, y, z, w;   // Position
	float r, g, b, a;   // Color
};

struct VertexPT
{
	float x, y, z, w;   // Position data
	float u, v;         // texture u,v
};


static const VertexPC coloredCubeData[] =
{
	// red face
	{ -1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	// green face
	{ -1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	// blue face
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	// yellow face
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f },
	// magenta face
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f },
	// cyan face
	{  1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f },
};

static const VertexPT texturedCubeData[] =
{
	// left face
	{ -1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 1.0f },
	// front face
	{ -1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 0.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 0.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f },
	// top face
	{ -1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f },
	// bottom face
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f },
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 0.0f },
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 1.0f },
	// right face
	{  1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f },
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 0.0f },
	// back face
	{ -1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 1.0f },
	{  1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f },
	{  1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f },
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	printf("Validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

vk::ShaderModule loadShaderModule(const char* filePath, const vk::Device& device) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("cannot open shader file");

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);
	file.close();

	vk::ShaderModuleCreateInfo shaderModuleInfo(
		vk::ShaderModuleCreateFlags(), buffer
	);

	printf("Shader module %s successfully load\n", filePath);

	return device.createShaderModule(shaderModuleInfo);
}

// copy-paste this functions from khronos repo

uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask) {
	uint32_t typeIndex = uint32_t(~0);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) &&
			((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask))
		{
			typeIndex = i;
			break;
		}
		typeBits >>= 1;
	}
	assert(typeIndex != uint32_t(~0));
	return typeIndex;
}

void submitAndWait(vk::Device const& device, vk::Queue const& queue, vk::CommandBuffer const& commandBuffer) {
	vk::Fence fence = device.createFence(vk::FenceCreateInfo());
	queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer), fence);
	while (vk::Result::eTimeout == device.waitForFences(fence, VK_TRUE, 1000000000))
		;
	device.destroyFence(fence);
}

int main(int argc, char* argv[]) {

	Window* rendererWindow = Window::CreateWindow("Vulkan Renderer", 1024, 768);
	SDL_Event event;
	
	std::unique_ptr<VulkanBase> base = std::make_unique<VulkanBase>();
	base->init(rendererWindow->GetWindow());

	// TODO: read about Extent2D
	vk::Extent2D swapchainExtent = base->GetSurfaceCapabilities().currentExtent;

	// TODO: clarify present modes
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

	// TODO: what is it?
	vk::SurfaceTransformFlagBitsKHR preTransform = (base->GetSurfaceCapabilities().supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: base->GetSurfaceCapabilities().currentTransform;

	vk::CompositeAlphaFlagBitsKHR compositeAlpha = (base->GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
		: (base->GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
		: (base->GetSurfaceCapabilities().supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit
		: vk::CompositeAlphaFlagBitsKHR::eOpaque;

	// TODO: find more informations about parts of this struct
	vk::SwapchainCreateInfoKHR swapchainInfo(
		vk::SwapchainCreateFlagsKHR(),
		base->GetSurface(),
		base->GetSurfaceCapabilities().minImageCount,
		base->GetFormat(),
		vk::ColorSpaceKHR::eSrgbNonlinear,
		swapchainExtent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		{},
		preTransform,
		compositeAlpha,
		swapchainPresentMode,
		true,
		nullptr
	);
	
	vk::SwapchainKHR swapchain = base->GetDevice().createSwapchainKHR(swapchainInfo);

	std::vector<vk::Image> swapchainImages = base->GetDevice().getSwapchainImagesKHR(swapchain);
	std::vector<vk::ImageView> imageViews;
	imageViews.reserve(swapchainImages.size());

	// TODO: read about componentMapping and ImageSubresourceRange
	vk::ComponentMapping componentMapping(
		vk::ComponentSwizzle::eR,
		vk::ComponentSwizzle::eG,
		vk::ComponentSwizzle::eB,
		vk::ComponentSwizzle::eA
	);

	vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	for (const auto& image : swapchainImages) {
		vk::ImageViewCreateInfo imageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			image,
			vk::ImageViewType::e2D,
			base->GetFormat(),
			componentMapping,
			subResourceRange
		);
		imageViews.push_back(base->GetDevice().createImageView(imageViewCreateInfo));
	}

	const vk::Format depthFormat = vk::Format::eD16Unorm;
	vk::FormatProperties formatProperties = base->GetPhysicalDevice().getFormatProperties(depthFormat);

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
		depthFormat,
		vk::Extent3D(swapchainExtent, 1),
		1, 1,
		vk::SampleCountFlagBits::e1,
		tiling,
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	);

	vk::Image depthImage = base->GetDevice().createImage(imageInfo);

	// TODO: Dealing with memory allocation in Vulkan
	vk::PhysicalDeviceMemoryProperties memoryProperties = base->GetPhysicalDevice().getMemoryProperties();
	vk::MemoryRequirements memoryRequirements = base->GetDevice().getImageMemoryRequirements(depthImage);
	uint32_t typeBits = memoryRequirements.memoryTypeBits;
	uint32_t typeIndex = findMemoryType(memoryProperties, typeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	// This part probably clear
	vk::DeviceMemory depthMemory = base->GetDevice().allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));
	base->GetDevice().bindImageMemory(depthImage, depthMemory, 0);

	// TODO: what is component mapping and subresource range (2)
	vk::ImageSubresourceRange depthSubResourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1);

	vk::ImageView depthView = base->GetDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			depthImage,
			vk::ImageViewType::e2D,
			depthFormat, 
			componentMapping,
			depthSubResourceRange
		)
	);

	// Uniform buffer data
	glm::mat4x4 model = glm::mat4x4(1.0f);
	glm::mat4x4 view = glm::lookAt(
		glm::vec3(-5.0f, 3.0f, -10.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f)
	);
	glm::mat4x4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4x4 clip = glm::mat4x4(
		1.0f,  0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f,  0.0f, 0.5f, 0.0f,
		0.0f,  0.0f, 0.5f, 1.0f
	);
	glm::mat4x4 mvpc = clip * projection * view * model;

	vk::Buffer uniformDataBuffer = base->GetDevice().createBuffer(
		vk::BufferCreateInfo(vk::BufferCreateFlags(), sizeof(mvpc), vk::BufferUsageFlagBits::eUniformBuffer)
	);
	
	vk::MemoryRequirements uniformMemoryReq = base->GetDevice().getBufferMemoryRequirements(uniformDataBuffer);
	uint32_t uniformTypeIndex = findMemoryType(
		memoryProperties, 
		uniformMemoryReq.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	vk::DeviceMemory uniformDataMemory = base->GetDevice().allocateMemory(vk::MemoryAllocateInfo(uniformMemoryReq.size, uniformTypeIndex));

	uint8_t* data = static_cast<uint8_t*>(base->GetDevice().mapMemory(uniformDataMemory, 0, uniformMemoryReq.size));
	memcpy(data, &mvpc, sizeof(mvpc));
	base->GetDevice().unmapMemory(uniformDataMemory);

	base->GetDevice().bindBufferMemory(uniformDataBuffer, uniformDataMemory, 0);

	// TODO: make sure what is descriptor set need for
	// Descriptor set stuff and pipelineLayout
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
		/* bindings ?? */ 0,
		vk::DescriptorType::eUniformBuffer,
		/* descriptor count */ 1,
		vk::ShaderStageFlagBits::eVertex
	);

	vk::DescriptorSetLayout descriptorSetLayout = base->GetDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), descriptorSetLayoutBinding)
	);
	
	vk::PipelineLayout pipelineLayout = base->GetDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout)
	);

	// Descriptor pool loool
	vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, 1);
	vk::DescriptorPool descriptorPool = base->GetDevice().createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			/* maxSets */1, 
			poolSize
		)
	);

	// allocate a desriptor set
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, descriptorSetLayout);
	vk::DescriptorSet descriptorSet = base->GetDevice().allocateDescriptorSets(descriptorSetAllocateInfo).front();

	// descriptor buffer (what a....)
	vk::DescriptorBufferInfo descriptorBufferInfo(
		uniformDataBuffer,
		/* offset */ 0,
		/* range  */ sizeof(glm::mat4x4)
	);

	vk::WriteDescriptorSet writeDescriptorSet(
		descriptorSet,
		0,
		0,
		vk::DescriptorType::eUniformBuffer,
		{},
		descriptorBufferInfo
	);
	base->GetDevice().updateDescriptorSets(writeDescriptorSet, nullptr);

	// TODO: read about attachment description
	std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
	attachmentDescriptions[0] = vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(),
		base->GetFormat(),
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

	vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass(
		vk::SubpassDescriptionFlags(),
		vk::PipelineBindPoint::eGraphics,
		{},
		colorReference,
		{},
		& depthReference
	);

	vk::RenderPass renderPass = base->GetDevice().createRenderPass(
		vk::RenderPassCreateInfo(
			vk::RenderPassCreateFlags(),
			attachmentDescriptions,
			subpass
		)
	);


	// shader modules stuff
	vk::ShaderModule cubeVertexShaderModule = loadShaderModule("../shaders/cube.vert.spv", base->GetDevice());
	vk::ShaderModule cubeFragmentShaderModule = loadShaderModule("../shaders/cube.frag.spv", base->GetDevice());

	// frame buffer stuff
	std::array<vk::ImageView, 2> attachments;
	attachments[1] = depthView;

	vk::FramebufferCreateInfo frameBufferCreateInfo(
		vk::FramebufferCreateFlags(),
		renderPass,
		attachments,
		swapchainExtent.width,
		swapchainExtent.height,
		1
	);

	std::vector<vk::Framebuffer> framebuffers;
	framebuffers.reserve(imageViews.size());
	for (const auto& imageView : imageViews) {
		attachments[0] = imageView;
		framebuffers.push_back(base->GetDevice().createFramebuffer(frameBufferCreateInfo));
	}

	// command pool and command buffer
	vk::CommandPool commandPool = base->GetDevice().createCommandPool(
		vk::CommandPoolCreateInfo(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			base->GetQueues().graphicsQueueIndex
		)
	);

	vk::CommandBuffer commandBuffer = base->GetDevice().allocateCommandBuffers(vk::CommandBufferAllocateInfo(
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1)
	).front();

	// vertex buffer
	vk::Buffer vertexBuffer = base->GetDevice().createBuffer(
		vk::BufferCreateInfo(
			vk::BufferCreateFlags(),
			sizeof(coloredCubeData),
			vk::BufferUsageFlagBits::eVertexBuffer
		)
	);

	vk::MemoryRequirements bufferMemoryRequiremenents = base->GetDevice().getBufferMemoryRequirements(vertexBuffer);
	uint32_t memoryTypeIndex = findMemoryType(base->GetPhysicalDevice().getMemoryProperties(), bufferMemoryRequiremenents.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	vk::DeviceMemory deviceMemory = base->GetDevice().allocateMemory(vk::MemoryAllocateInfo(bufferMemoryRequiremenents.size, memoryTypeIndex));

	data = static_cast<uint8_t*>(base->GetDevice().mapMemory(deviceMemory, 0, bufferMemoryRequiremenents.size));
	memcpy(data, coloredCubeData, sizeof(coloredCubeData));
	base->GetDevice().unmapMemory(deviceMemory);

	base->GetDevice().bindBufferMemory(vertexBuffer, deviceMemory, 0);

	// pipeline 
	std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStagesCreateInfos = {
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			cubeVertexShaderModule,
			"main"
		),
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			cubeFragmentShaderModule,
			"main"
		)
	};

	vk::VertexInputBindingDescription vertexInputBindingDescription(
		0,
		sizeof(coloredCubeData[0])
	);

	std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributeDescriptions = {
		vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, 0),
		vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, 16)
	};

	vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
		vk::PipelineVertexInputStateCreateFlags(),  // flags
		vertexInputBindingDescription,              // vertexBindingDescriptions
		vertexInputAttributeDescriptions            // vertexAttributeDescriptions
	);

	vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyCreateInfo(
		vk::PipelineInputAssemblyStateCreateFlags(),
		vk::PrimitiveTopology::eTriangleList
	);

	vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
		vk::PipelineViewportStateCreateFlags(),
		1,
		nullptr,
		1,
		nullptr
	);

	vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
		vk::PipelineRasterizationStateCreateFlags(),  // flags
		false,                                        // depthClampEnable
		false,                                        // rasterizerDiscardEnable
		vk::PolygonMode::eFill,                       // polygonMode
		vk::CullModeFlagBits::eBack,                  // cullMode
		vk::FrontFace::eClockwise,                    // frontFace
		false,                                        // depthBiasEnable
		0.0f,                                         // depthBiasConstantFactor
		0.0f,                                         // depthBiasClamp
		0.0f,                                         // depthBiasSlopeFactor
		1.0f                                          // lineWidth
	);

	vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
		vk::PipelineMultisampleStateCreateFlags(),
		vk::SampleCountFlagBits::e1
	);

	vk::StencilOpState stencilOpState(
		vk::StencilOp::eKeep,
		vk::StencilOp::eKeep,
		vk::StencilOp::eKeep,
		vk::CompareOp::eAlways
	);

	vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
		vk::PipelineDepthStencilStateCreateFlags(),  // flags
		true,                                        // depthTestEnable
		true,                                        // depthWriteEnable
		vk::CompareOp::eLessOrEqual,                 // depthCompareOp
		false,                                       // depthBoundTestEnable
		false,                                       // stencilTestEnable
		stencilOpState,                              // front
		stencilOpState                               // back
	);

	vk::ColorComponentFlags colorComponentFlags(
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	);

	vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
		false,                   // blendEnable
		vk::BlendFactor::eZero,  // srcColorBlendFactor
		vk::BlendFactor::eZero,  // dstColorBlendFactor
		vk::BlendOp::eAdd,       // colorBlendOp
		vk::BlendFactor::eZero,  // srcAlphaBlendFactor
		vk::BlendFactor::eZero,  // dstAlphaBlendFactor
		vk::BlendOp::eAdd,       // alphaBlendOp
		colorComponentFlags      // colorWriteMask
	);

	vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
		vk::PipelineColorBlendStateCreateFlags(),  // flags
		false,                                     // logicOpEnable
		vk::LogicOp::eNoOp,                        // logicOp
		pipelineColorBlendAttachmentState,         // attachments
		{ { 1.0f, 1.0f, 1.0f, 1.0f } }             // blendConstants
	);

	std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
		vk::PipelineDynamicStateCreateFlags(),
		dynamicStates
	);

	vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
		vk::PipelineCreateFlags(),
		pipelineShaderStagesCreateInfos,
		&pipelineVertexInputStateCreateInfo,
		&pipelineInputAssemblyCreateInfo,
		nullptr,
		&pipelineViewportStateCreateInfo,
		&pipelineRasterizationStateCreateInfo,
		&pipelineMultisampleStateCreateInfo,
		&pipelineDepthStencilStateCreateInfo,
		&pipelineColorBlendStateCreateInfo,
		&pipelineDynamicStateCreateInfo,
		pipelineLayout,
		renderPass
	);

	vk::Result result;
	vk::Pipeline pipeline;
	std::tie(result, pipeline) = base->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);


	uint32_t frameNumber = 0;
	while (!rendererWindow->isShouldClose) {
		while (rendererWindow->PollEvents(&event)) {
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
				rendererWindow->isShouldClose = true;
		}

		// Not sure that this is part of vertex buffer stuff
		vk::Semaphore imageAcquiredSemaphore = base->GetDevice().createSemaphore(vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags()));

		vk::ResultValue<uint32_t> currentBuffer = base->GetDevice().acquireNextImageKHR(
			swapchain,
			1000000000,
			imageAcquiredSemaphore,
			nullptr
		);
		assert(currentBuffer.result == vk::Result::eSuccess);
		assert(currentBuffer.value < framebuffers.size());

		std::array<vk::ClearValue, 2> clearValues;
		// random colors to window color
		srand(time(nullptr));
		float redFlash = abs(sin(rand() / 120.0f));
		float greenFlash = abs(sin(rand() / 120.0f));
		float blueFlash = abs(sin(rand() / 120.0f));
		float alphaFlash = abs(sin(rand() / 120.0f));
		clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ {redFlash, greenFlash, blueFlash, alphaFlash} }));
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		// Uniform buffer data
		model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 0.7f), glm::vec3(1.0f, 0.3f, 0.5f));
		view = glm::lookAt(
			glm::vec3(-5.0f, 3.0f, -10.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		);
		projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
		clip = glm::mat4x4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f
		);
		mvpc = clip * projection * view * model;
		++frameNumber;

		data = static_cast<uint8_t*>(base->GetDevice().mapMemory(uniformDataMemory, 0, uniformMemoryReq.size));
		memcpy(data, &mvpc, sizeof(mvpc));
		base->GetDevice().unmapMemory(uniformDataMemory);

		commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

		vk::RenderPassBeginInfo renderPassBeginInfo(
			renderPass,
			framebuffers[currentBuffer.value],
			vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent),
			clearValues
		);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		// ???
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);

		commandBuffer.bindVertexBuffers(0, vertexBuffer, { 0 });

		commandBuffer.setViewport(
			0,
			vk::Viewport(
				0.0f, 0.0f,
				static_cast<float>(swapchainExtent.width),
				static_cast<float>(swapchainExtent.height),
				0.0f,
				1.0f
			)
		);

		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent));

		commandBuffer.draw(12 * 3, 1, 0, 0);

		commandBuffer.endRenderPass();
		commandBuffer.end();

		// rewrite
		//submitAndWait(device, graphicsQueue, commandBuffer);
		
		// submiting to queue and present
		vk::Fence drawFence = base->GetDevice().createFence(vk::FenceCreateInfo());
		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo submitInfo(
			imageAcquiredSemaphore,
			waitDestinationStageMask,
			commandBuffer
		);
		base->GetQueues().graphicsQueue.submit(submitInfo, drawFence);

		while (vk::Result::eTimeout == base->GetDevice().waitForFences(drawFence, VK_TRUE, 1000000000));

		vk::Result drawResult = base->GetQueues().graphicsQueue.presentKHR(vk::PresentInfoKHR({}, swapchain, currentBuffer.value));

		base->GetDevice().waitIdle();

		base->GetDevice().destroyFence(drawFence);
		base->GetDevice().destroySemaphore(imageAcquiredSemaphore);
	}
	
	base->GetDevice().destroyPipeline(pipeline);

	base->GetDevice().freeMemory(deviceMemory);
	base->GetDevice().destroyBuffer(vertexBuffer);

	base->GetDevice().freeCommandBuffers(commandPool, commandBuffer);
	base->GetDevice().destroyCommandPool(commandPool);

	for (const auto& framebuffer : framebuffers) {
		base->GetDevice().destroyFramebuffer(framebuffer);
	}

	base->GetDevice().destroyShaderModule(cubeFragmentShaderModule);
	base->GetDevice().destroyShaderModule(cubeVertexShaderModule);
	
	base->GetDevice().destroyRenderPass(renderPass);

	base->GetDevice().freeDescriptorSets(descriptorPool, descriptorSet);
	base->GetDevice().destroyDescriptorPool(descriptorPool);

	base->GetDevice().destroyPipelineLayout(pipelineLayout);
	base->GetDevice().destroyDescriptorSetLayout(descriptorSetLayout);

	base->GetDevice().freeMemory(uniformDataMemory);
	base->GetDevice().destroyBuffer(uniformDataBuffer);
	base->GetDevice().destroyImageView(depthView);
	base->GetDevice().freeMemory(depthMemory);
	base->GetDevice().destroyImage(depthImage);
	for (auto& imageView : imageViews) {
		base->GetDevice().destroyImageView(imageView);
	}
	base->GetDevice().destroySwapchainKHR(swapchain);
	//base->instance.destroySurfaceKHR(base->surface);
	//base->device.destroy();
	////base->instance.destroyDebugUtilsMessengerEXT(base->debugMessenger, nullptr, base->dispatcher);
	//base->instance.destroy();
	////SDL_DestroyWindow(window);

	return 0;
}
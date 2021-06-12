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

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
		"VulkanEngine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1024, 768,
		SDL_WINDOW_VULKAN
	);

	SDL_Event event;
	bool isShouldClose = false;

	std::vector<const char*> layerProperties{
		"VK_LAYER_KHRONOS_validation"
	};

	uint32_t sdlExtensionsCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, nullptr);
	std::vector<const char*> instanceExtensions = {
		// Extension VK_KHR_surface will get by SDL and I think it doesnt need it vector 
		//"VK_KHR_surface",

		// TODO: Find differences in utils and report
		"VK_EXT_debug_report",
		"VK_EXT_debug_utils"
	};
	size_t additionalExtensionsSize = instanceExtensions.size();
	instanceExtensions.resize(instanceExtensions.size() + sdlExtensionsCount);
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, instanceExtensions.data() + additionalExtensionsSize);

	vk::ApplicationInfo appInfo = { "VulkanEngineApp", 1, "VulkanEngine", 1, VK_API_VERSION_1_1 };
	vk::InstanceCreateInfo instanceInfo = { {}, &appInfo };
	instanceInfo.enabledLayerCount = layerProperties.size();
	instanceInfo.ppEnabledLayerNames = layerProperties.data();
	instanceInfo.enabledExtensionCount = instanceExtensions.size();
	instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
	vk::Instance instance = vk::createInstance(instanceInfo);
	
	// TODO: Find some additional information about vk::DispatchLoaderDynamic
	const auto dispatcher = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

	// TODO: Read some info about debug messengers in Vulkan
	auto debugMessenger = instance.createDebugUtilsMessengerEXT(
		vk::DebugUtilsMessengerCreateInfoEXT{
			{},
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			debugCallback 
		},
		nullptr,
		dispatcher
	);

	vk::PhysicalDevice gpu;
	for (const auto& physicalDevice : instance.enumeratePhysicalDevices()) {
		vk::PhysicalDeviceProperties gpuProps = physicalDevice.getProperties();
		gpu = strstr(gpuProps.deviceName, "NVIDIA") != nullptr ? physicalDevice : nullptr;
	}

	VkSurfaceKHR vksurface;
	SDL_bool res = SDL_Vulkan_CreateSurface(window, instance, &vksurface);
	vk::SurfaceKHR surface(vksurface);

	std::vector<vk::QueueFamilyProperties> queueFamilyProps = gpu.getQueueFamilyProperties();
	const auto graphicProp = std::find_if(queueFamilyProps.begin(), queueFamilyProps.end(),
		[](vk::QueueFamilyProperties qfp) {
		return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
	});
	size_t graphicsQueueFamilyIndex = std::distance(queueFamilyProps.begin(), graphicProp);

	// TODO: We have to find different index if it equals to queueFamilyProps.size()
	size_t presentQueueFamilyIndex = gpu.getSurfaceSupportKHR(static_cast<uint32_t>(graphicsQueueFamilyIndex), surface) 
		? graphicsQueueFamilyIndex
		: queueFamilyProps.size();

	float queuePriotiry = 1.0f;
	vk::DeviceQueueCreateInfo deviceQueueInfo(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(graphicsQueueFamilyIndex), 1, &queuePriotiry);

	std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk::Device device = gpu.createDevice(
		vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1u, &deviceQueueInfo, 
			0u, nullptr,
			static_cast<uint32_t>(deviceExtensions.size()),
			deviceExtensions.data()
		)
	);

	vk::Queue graphicsQueue = device.getQueue(graphicsQueueFamilyIndex, 0);

	std::vector<vk::SurfaceFormatKHR> formats = gpu.getSurfaceFormatsKHR(surface);

	// TODO: should check is there more than 1 formats and is it define
	vk::Format format = formats.front().format; // is this color format???

	// TODO: read about surface capabilities
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = gpu.getSurfaceCapabilitiesKHR(surface);

	// TODO: read about Extent2D
	vk::Extent2D swapchainExtent = surfaceCapabilities.currentExtent;

	// TODO: clarify present modes
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

	// TODO: what is it?
	vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: surfaceCapabilities.currentTransform;

	vk::CompositeAlphaFlagBitsKHR compositeAlpha = (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
		: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
		: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit
		: vk::CompositeAlphaFlagBitsKHR::eOpaque;

	std::vector<uint32_t> queueFamilyIndexes = {
		static_cast<uint32_t>(graphicsQueueFamilyIndex),
		static_cast<uint32_t>(presentQueueFamilyIndex)
	};

	const auto iter = std::unique(queueFamilyIndexes.begin(), queueFamilyIndexes.end());
	queueFamilyIndexes.erase(queueFamilyIndexes.begin(), iter);

	// TODO: find more informations about parts of this struct
	vk::SwapchainCreateInfoKHR swapchainInfo(
		vk::SwapchainCreateFlagsKHR(),
		surface ,
		surfaceCapabilities.minImageCount,
		format,
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
	
	vk::SwapchainKHR swapchain = device.createSwapchainKHR(swapchainInfo);

	std::vector<vk::Image> swapchainImages = device.getSwapchainImagesKHR(swapchain);
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
			format,
			componentMapping,
			subResourceRange
		);
		imageViews.push_back(device.createImageView(imageViewCreateInfo));
	}

	const vk::Format depthFormat = vk::Format::eD16Unorm;
	vk::FormatProperties formatProperties = gpu.getFormatProperties(depthFormat);

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

	vk::Image depthImage = device.createImage(imageInfo);

	// TODO: Dealing with memory allocation in Vulkan
	vk::PhysicalDeviceMemoryProperties memoryProperties = gpu.getMemoryProperties();
	vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(depthImage);
	uint32_t typeBits = memoryRequirements.memoryTypeBits;
	uint32_t typeIndex = findMemoryType(memoryProperties, typeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	// This part probably clear
	vk::DeviceMemory depthMemory = device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));
	device.bindImageMemory(depthImage, depthMemory, 0);

	// TODO: what is component mapping and subresource range (2)
	vk::ImageSubresourceRange depthSubResourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1);

	vk::ImageView depthView = device.createImageView(
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

	vk::Buffer uniformDataBuffer = device.createBuffer(
		vk::BufferCreateInfo(vk::BufferCreateFlags(), sizeof(mvpc), vk::BufferUsageFlagBits::eUniformBuffer)
	);
	
	vk::MemoryRequirements uniformMemoryReq = device.getBufferMemoryRequirements(uniformDataBuffer);
	uint32_t uniformTypeIndex = findMemoryType(
		memoryProperties, 
		uniformMemoryReq.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	vk::DeviceMemory uniformDataMemory = device.allocateMemory(vk::MemoryAllocateInfo(uniformMemoryReq.size, uniformTypeIndex));

	uint8_t* data = static_cast<uint8_t*>(device.mapMemory(uniformDataMemory, 0, uniformMemoryReq.size));
	memcpy(data, &mvpc, sizeof(mvpc));
	device.unmapMemory(uniformDataMemory);

	device.bindBufferMemory(uniformDataBuffer, uniformDataMemory, 0);

	// TODO: make sure what is descriptor set need for
	// Descriptor set stuff and pipelineLayout
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
		/* bindings ?? */ 0,
		vk::DescriptorType::eUniformBuffer,
		/* descriptor count */ 1,
		vk::ShaderStageFlagBits::eVertex
	);

	vk::DescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), descriptorSetLayoutBinding)
	);
	
	vk::PipelineLayout pipelineLayout = device.createPipelineLayout(
		vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout)
	);

	// Descriptor pool loool
	vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, 1);
	vk::DescriptorPool descriptorPool = device.createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			/* maxSets */1, 
			poolSize
		)
	);

	// allocate a desriptor set
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, descriptorSetLayout);
	vk::DescriptorSet descriptorSet = device.allocateDescriptorSets(descriptorSetAllocateInfo).front();

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
	device.updateDescriptorSets(writeDescriptorSet, nullptr);

	// TODO: read about attachment description
	std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
	attachmentDescriptions[0] = vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(),
		format,
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

	vk::RenderPass renderPass = device.createRenderPass(
		vk::RenderPassCreateInfo(
			vk::RenderPassCreateFlags(),
			attachmentDescriptions,
			subpass
		)
	);


	// shader modules stuff
	vk::ShaderModule cubeVertexShaderModule = loadShaderModule("../shaders/cube.vert.spv", device);
	vk::ShaderModule cubeFragmentShaderModule = loadShaderModule("../shaders/cube.frag.spv", device);

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
		framebuffers.push_back(device.createFramebuffer(frameBufferCreateInfo));
	}

	// command pool and command buffer
	vk::CommandPool commandPool = device.createCommandPool(
		vk::CommandPoolCreateInfo(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			graphicsQueueFamilyIndex
		)
	);

	vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1)
	).front();

	// vertex buffer
	vk::Buffer vertexBuffer = device.createBuffer(
		vk::BufferCreateInfo(
			vk::BufferCreateFlags(),
			sizeof(coloredCubeData),
			vk::BufferUsageFlagBits::eVertexBuffer
		)
	);

	vk::MemoryRequirements bufferMemoryRequiremenents = device.getBufferMemoryRequirements(vertexBuffer);
	uint32_t memoryTypeIndex = findMemoryType(gpu.getMemoryProperties(), bufferMemoryRequiremenents.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	vk::DeviceMemory deviceMemory = device.allocateMemory(vk::MemoryAllocateInfo(bufferMemoryRequiremenents.size, memoryTypeIndex));

	data = static_cast<uint8_t*>(device.mapMemory(deviceMemory, 0, bufferMemoryRequiremenents.size));
	memcpy(data, coloredCubeData, sizeof(coloredCubeData));
	device.unmapMemory(deviceMemory);

	device.bindBufferMemory(vertexBuffer, deviceMemory, 0);

	while (!isShouldClose) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
				isShouldClose = true;
		}

		// Not sure that this is part of vertex buffer stuff
		vk::Semaphore imageAcquiredSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags()));

		vk::ResultValue<uint32_t> currentBuffer = device.acquireNextImageKHR(
			swapchain,
			1000000000,
			imageAcquiredSemaphore,
			nullptr
		);
		assert(currentBuffer.result == vk::Result::eSuccess);
		assert(currentBuffer.value < framebuffers.size());

		std::array<vk::ClearValue, 2> clearValues;
		srand(time(nullptr));
		float redFlash = abs(sin(rand() / 120.0f));
		float greenFlash = abs(sin(rand() / 120.0f));
		float blueFlash = abs(sin(rand() / 120.0f));
		float alphaFlash = abs(sin(rand() / 120.0f));
		clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ {redFlash, greenFlash, blueFlash, alphaFlash} }));
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

		vk::RenderPassBeginInfo renderPassBeginInfo(
			renderPass,
			framebuffers[currentBuffer.value],
			vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent),
			clearValues
		);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		
		commandBuffer.bindVertexBuffers(0, vertexBuffer, { 0 });

		commandBuffer.endRenderPass();
		commandBuffer.end();

		submitAndWait(device, graphicsQueue, commandBuffer);
		graphicsQueue.presentKHR(vk::PresentInfoKHR({}, swapchain, currentBuffer.value));
		device.destroySemaphore(imageAcquiredSemaphore);
	}
	
	device.freeMemory(deviceMemory);
	device.destroyBuffer(vertexBuffer);

	device.freeCommandBuffers(commandPool, commandBuffer);
	device.destroyCommandPool(commandPool);

	for (const auto& framebuffer : framebuffers) {
		device.destroyFramebuffer(framebuffer);
	}

	device.destroyShaderModule(cubeFragmentShaderModule);
	device.destroyShaderModule(cubeVertexShaderModule);
	
	device.destroyRenderPass(renderPass);

	device.freeDescriptorSets(descriptorPool, descriptorSet);
	device.destroyDescriptorPool(descriptorPool);

	device.destroyPipelineLayout(pipelineLayout);
	device.destroyDescriptorSetLayout(descriptorSetLayout);

	device.freeMemory(uniformDataMemory);
	device.destroyBuffer(uniformDataBuffer);
	device.destroyImageView(depthView);
	device.freeMemory(depthMemory);
	device.destroyImage(depthImage);
	for (auto& imageView : imageViews) {
		device.destroyImageView(imageView);
	}
	device.destroySwapchainKHR(swapchain);
	instance.destroySurfaceKHR(surface);
	device.destroy();
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dispatcher);
	instance.destroy();
	SDL_DestroyWindow(window);

	return 0;
}

#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>

#include <VkBootstrap.h>

#include <iostream>
#include <fstream>

#define VK_CHECK(x)                                                     \
	do {                                                                \
		VkResult err = x;                                               \
		if (err) {                                                      \
			std::cout << "Detected Vulkan Error: " << err << std::endl; \
			abort();                                                    \
		}                                                               \
	} while (0)	                                                        \

VulkanEngine::VulkanEngine() {
	isInitialized = false;
	frameNumber = 0;
	selectedShader = 0;
	windowExtent = { 800, 600 };
}

void VulkanEngine::init()
{
	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN);
	
	window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowExtent.width,
		windowExtent.height,
		window_flags
	);
	
	InitVulkan();
	InitSwapchain();
	InitDefaultRenderpass();
	InitFramebuffers();
	InitCommands();
	InitSyncStructures();
	InitPipelines();

	//everything went fine
	isInitialized = true;
}

void VulkanEngine::cleanup()
{	
	if (isInitialized) {
		
		vkDeviceWaitIdle(device);

		mainDeletionQueue.flush();

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);
		vkb::destroy_debug_utils_messenger(instance, debugMessenger);
		vkDestroyInstance(instance, nullptr);
		SDL_DestroyWindow(window);
	}
}

void VulkanEngine::draw() {
	VK_CHECK(vkWaitForFences(device, 1, &renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(device, 1, &renderFence));
	VK_CHECK(vkResetCommandBuffer(mainCommandBuffer, 0));

	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex));

	VkCommandBuffer cmd = mainCommandBuffer;
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;
	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	VkClearValue clearValue;
	float flash = abs(sin(frameNumber / 120.f));
	clearValue.color = { {0.0f, flash, flash, 1.0f} };

	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;
	rpInfo.renderPass = renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = windowExtent;
	rpInfo.framebuffer = framebuffers[swapchainImageIndex];
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	if (selectedShader == 0)
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
	else 
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, redTrianglePipeline);

	vkCmdDraw(cmd, 3, 1, 0, 0);

	vkCmdEndRenderPass(cmd);
	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit.pWaitDstStageMask = &waitStage;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &presentSemaphore;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &renderSemaphore;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;

	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, renderFence));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

	++frameNumber;
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT) bQuit = true;
 			if (e.type == SDL_KEYDOWN) {
				std::cout << (char)e.key.keysym.sym << " Key pressed\n";
				if (e.key.keysym.sym == SDLK_SPACE) {
					selectedShader += 1;
					if (selectedShader > 1){
						selectedShader = 0;
					}
				}
			}
		}

		draw();
	}
}

void VulkanEngine::InitVulkan() {
	vkb::InstanceBuilder builder;

	// make vulkan instance with basic debug features
	auto instRet = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(true)
		.use_default_debug_messenger()
		.require_api_version(1, 1, 0)
		.build();

	vkb::Instance inst = instRet.value();

	instance = inst.instance;
	debugMessenger = inst.debug_messenger;

	SDL_Vulkan_CreateSurface(window, instance, &surface);

	vkb::PhysicalDeviceSelector selector{ inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(surface)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

	device = vkbDevice.device;
	gpu = physicalDevice.physical_device;

	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::InitSwapchain() {
	vkb::SwapchainBuilder builder{ gpu, device, surface };
	vkb::Swapchain vkbSwapchain = builder
		.use_default_format_selection()
		// use vSync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(windowExtent.width, windowExtent.height)
		.build()
		.value();

	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
	swapchainImageFormat = vkbSwapchain.image_format;

	mainDeletionQueue.pushFunction([=]() {
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	});
}

void VulkanEngine::InitCommands() {
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(commandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer));

	mainDeletionQueue.pushFunction([=]() {
		vkDestroyCommandPool(device, commandPool, nullptr);
	});
}

void VulkanEngine::InitDefaultRenderpass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	mainDeletionQueue.pushFunction([=]() {
		vkDestroyRenderPass(device, renderPass, nullptr);
	});
}

void VulkanEngine::InitFramebuffers() {
	VkFramebufferCreateInfo fbInfo = {};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = 1;
	fbInfo.width = windowExtent.width;
	fbInfo.height = windowExtent.height;
	fbInfo.layers = 1;

	const uint32_t swapchainImageCount = swapchainImages.size();
	framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; ++i) {
		fbInfo.pAttachments = &swapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffers[i]));

		mainDeletionQueue.pushFunction([=]() {
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		});
	}
}

void VulkanEngine::InitSyncStructures() {
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));

	mainDeletionQueue.pushFunction([=]() {
		vkDestroyFence(device, renderFence, nullptr);
	});

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore));
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore));

	mainDeletionQueue.pushFunction([=]() {
		vkDestroySemaphore(device, presentSemaphore, nullptr);
		vkDestroySemaphore(device, renderSemaphore, nullptr);
	});
}

void VulkanEngine::InitPipelines() {
	VkShaderModule triangleFragShader;
	if (!LoadShaderModule("../shaders/coloredTriangle.frag.spv", &triangleFragShader))
		std::cout << "Error when building the colored triangle fragment shader module";
	else
		std::cout << "Colored Triangle fragment shader succesfully loaded" << std::endl;

	VkShaderModule triangleVertexShader;
	if (!LoadShaderModule("../shaders/coloredTriangle.vert.spv", &triangleVertexShader))
		std::cout << "Error when building the colored triangle vertex shader module";
	else
		std::cout << "Colored Triangle vertex shader succesfully loaded" << std::endl;

	VkShaderModule redTriangleFragShader;
	if (!LoadShaderModule("../shaders/triangle.frag.spv", &redTriangleFragShader))
		std::cout << "Error when building the red triangle fragment shader module";
	else
		std::cout << "Red Triangle fragment shader succesfully loaded" << std::endl;

	VkShaderModule redTriangleVertexShader;
	if (!LoadShaderModule("../shaders/triangle.vert.spv", &redTriangleVertexShader))
		std::cout << "Error when building the red triangle vertex shader module";
	else
		std::cout << "Red Triangle vertex shader succesfully loaded" << std::endl;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo();
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &trianglePipelineLayout));

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader));
	pipelineBuilder.shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));

	pipelineBuilder.vertexInputInfo = vkinit::VertexInputStateCreateInfo();
	pipelineBuilder.inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	pipelineBuilder.viewport.x = 0.0f;
	pipelineBuilder.viewport.y = 0.0f;
	pipelineBuilder.viewport.width = (float)windowExtent.width;
	pipelineBuilder.viewport.height = (float)windowExtent.height;
	pipelineBuilder.viewport.minDepth = 0.0f;
	pipelineBuilder.viewport.maxDepth = 1.0f;

	pipelineBuilder.scissor.offset = { 0, 0 };
	pipelineBuilder.scissor.extent = windowExtent;

	pipelineBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	pipelineBuilder.multisampling = vkinit::MultisamplingStateCreateInfo();
	pipelineBuilder.colorBlendingAttachment = vkinit::ColorBlendAttachmentState();
	
	pipelineBuilder.pipelinaLayout = trianglePipelineLayout;

	trianglePipeline = pipelineBuilder.Build(device, renderPass);

	pipelineBuilder.shaderStages.clear();
	pipelineBuilder.shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, redTriangleVertexShader));
	pipelineBuilder.shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, redTriangleFragShader));

	redTrianglePipeline = pipelineBuilder.Build(device, renderPass);

	vkDestroyShaderModule(device, redTriangleVertexShader, nullptr);
	vkDestroyShaderModule(device, redTriangleFragShader, nullptr);
	vkDestroyShaderModule(device, triangleVertexShader, nullptr);
	vkDestroyShaderModule(device, triangleFragShader, nullptr);

	mainDeletionQueue.pushFunction([=]() {
		vkDestroyPipeline(device, redTrianglePipeline, nullptr);
		vkDestroyPipeline(device, trianglePipeline, nullptr);
		vkDestroyPipelineLayout(device, trianglePipelineLayout, nullptr);
	});
}

bool VulkanEngine::LoadShaderModule(const char* filePath, VkShaderModule* outShaderModule) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return false;

	size_t fileSize = (size_t)file.tellg();
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);
	file.close();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		return false;
	}

	*outShaderModule = shaderModule;
	return true;
}

VkPipeline PipelineBuilder::Build(VkDevice device, VkRenderPass pass) {
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendingAttachment;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelinaLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		std::cout << "failed to create pipeline";
		return VK_NULL_HANDLE;
	} else {
		return newPipeline;
	}
}
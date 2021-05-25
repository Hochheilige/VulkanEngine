
#include <vk_engine.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>

#include <VkBootstrap.h>

#include <iostream>
#include <fstream>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

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
	window = nullptr;
	frames.resize(FRAME_OVERLAP);
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
	InitDescriptors();
	InitPipelines();

	LoadMeshes();

	InitScene();

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

FrameData& VulkanEngine::GetCurrentFrame() {
	return frames[frameNumber % FRAME_OVERLAP];
}

void VulkanEngine::draw() {
	VK_CHECK(vkWaitForFences(device, 1, &GetCurrentFrame().renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(device, 1, &GetCurrentFrame().renderFence));
	VK_CHECK(vkResetCommandBuffer(GetCurrentFrame().mainCommandBuffer, 0));

	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, GetCurrentFrame().presentSemaphore, nullptr, &swapchainImageIndex));

	VkCommandBuffer cmd = GetCurrentFrame().mainCommandBuffer;
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	VkClearValue clearValue;
	float flash = abs(sin(frameNumber / 120.f));
	clearValue.color = { {0.0f, flash, flash, 1.0f} };

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1;

	VkClearValue clearValues[] = {
		clearValue,
		depthClear
	};

	VkRenderPassBeginInfo rpInfo = vkinit::RenderPassBeginInfo(renderPass, windowExtent, framebuffers[swapchainImageIndex]);
	rpInfo.clearValueCount = 2;
	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	DrawObjects(cmd, renderables.data(), renderables.size());
	
	vkCmdEndRenderPass(cmd);
	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit.pWaitDstStageMask = &waitStage;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &GetCurrentFrame().presentSemaphore;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &GetCurrentFrame().renderSemaphore;
	submit.commandBufferCount = 1;

	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, GetCurrentFrame().renderFence));

	VkPresentInfoKHR presentInfo = vkinit::PresentInfo();
	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
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
					if (selectedShader > 4){
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

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = gpu;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &allocator);

	mainDeletionQueue.pushFunction([&]() {
		vmaDestroyAllocator(allocator);
	});

	vkGetPhysicalDeviceProperties(gpu, &gpuProperties);
	std::cout << "The GPU has a minimum buffer aligment of " << gpuProperties.limits.minUniformBufferOffsetAlignment << std::endl;
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

	VkExtent3D depthImageExtent = {
		windowExtent.width,
		windowExtent.height,
		1
	};

	depthFormat = VK_FORMAT_D32_SFLOAT;

	VkImageCreateInfo dImgInfo = vkinit::ImageCreateInfo(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	VmaAllocationCreateInfo dImgAllocInfo = {};
	dImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dImgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(allocator, &dImgInfo, &dImgAllocInfo, &depthImage.image, &depthImage.allocation, nullptr);

	VkImageViewCreateInfo dViewInfo = vkinit::ImageviewCreateInfo(depthFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(device, &dViewInfo, nullptr, &depthImageView));

	mainDeletionQueue.pushFunction([=]() {
		vkDestroyImageView(device, depthImageView, nullptr);
		vmaDestroyImage(allocator, depthImage.image, depthImage.allocation);
	});
}

void VulkanEngine::InitCommands() {
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; ++i) {
		VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frames[i].commandPool));

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(frames[i].commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frames[i].mainCommandBuffer));

		mainDeletionQueue.pushFunction([=]() {
			vkDestroyCommandPool(device, frames[i].commandPool, nullptr);
		});
	}
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

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.flags = 0;
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	//1 dependency, which is from "outside" into the subpass. And we can read or write color
	// Пока что не очень ясная штука
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = &attachments[0];
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	mainDeletionQueue.pushFunction([=]() {
		vkDestroyRenderPass(device, renderPass, nullptr);
	});
}

void VulkanEngine::InitFramebuffers() {
	VkFramebufferCreateInfo fbInfo = vkinit::FramebufferCreateInfo(renderPass, windowExtent);

	const uint32_t swapchainImageCount = swapchainImages.size();
	framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; ++i) {
		VkImageView attachments[2];
		attachments[0] = swapchainImageViews[i];
		attachments[1] = depthImageView; 

		fbInfo.pAttachments = attachments;
		fbInfo.attachmentCount = 2;
		VK_CHECK(vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffers[i]));

		mainDeletionQueue.pushFunction([=]() {
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		});
	}
}

void VulkanEngine::InitSyncStructures() {
	VkFenceCreateInfo fenceCreateInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::SemaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; ++i) {
		VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence));

		mainDeletionQueue.pushFunction([=]() {
			vkDestroyFence(device, frames[i].renderFence, nullptr);
		});



		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore));
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore));

		mainDeletionQueue.pushFunction([=]() {
			vkDestroySemaphore(device, frames[i].presentSemaphore, nullptr);
			vkDestroySemaphore(device, frames[i].renderSemaphore, nullptr);
		});
	}
}

void VulkanEngine::InitPipelines() {
	VkShaderModule colorMeshShader;
	if (!LoadShaderModule("../shaders/defaultLit.frag.spv", &colorMeshShader))
		std::cout << "Error when building the colored triangle fragment shader module";
	else
		std::cout << "Colored Triangle fragment shader succesfully loaded" << std::endl;

	VkShaderModule meshVertShader;
	if (!LoadShaderModule("../shaders/triMesh.vert.spv", &meshVertShader))
		std::cout << "Error when building the mesh triangle vertex shader module";
	else
		std::cout << "Mesh Triangle vertex shader succesfully loaded" << std::endl;

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
	pipelineBuilder.shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colorMeshShader));

	VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo();
	VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(MeshPushConstants);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	meshPipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	meshPipelineLayoutInfo.pushConstantRangeCount = 1;

	meshPipelineLayoutInfo.setLayoutCount = 1;
	meshPipelineLayoutInfo.pSetLayouts = &globalSetLayout;

	VkPipelineLayout meshPipelineLayout;
	VK_CHECK(vkCreatePipelineLayout(device, &meshPipelineLayoutInfo, nullptr, &meshPipelineLayout));
	
	pipelineBuilder.pipelinaLayout = meshPipelineLayout;
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
	pipelineBuilder.depthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	VertexInputDescription vertexDescription = Vertex::GetVertexDescription();
	pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();
	pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	VkPipeline meshPipeline = pipelineBuilder.Build(device, renderPass);

	CreateMaterial(meshPipeline, meshPipelineLayout, "defaultmesh");

	vkDestroyShaderModule(device, meshVertShader, nullptr);
	vkDestroyShaderModule(device, colorMeshShader, nullptr);

	mainDeletionQueue.pushFunction([=]() {
		vkDestroyPipeline(device, meshPipeline, nullptr);
		vkDestroyPipelineLayout(device, meshPipelineLayout, nullptr);
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

void VulkanEngine::InitDescriptors() {
	std::vector<VkDescriptorPoolSize> sizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , 10}, 
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10}
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.maxSets = 10;
	poolInfo.poolSizeCount = (uint32_t)sizes.size();
	poolInfo.pPoolSizes = sizes.data();

	vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

	VkDescriptorSetLayoutBinding cameraBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
	VkDescriptorSetLayoutBinding sceneBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	VkDescriptorSetLayoutBinding bindings[] = { cameraBind, sceneBind };

	VkDescriptorSetLayoutCreateInfo setInfo = {};
	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setInfo.pNext = nullptr;
	setInfo.bindingCount = 2;
	setInfo.flags = 0;
	setInfo.pBindings = bindings;

	vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &globalSetLayout);

	VkDescriptorSetLayoutBinding objectBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	VkDescriptorSetLayoutCreateInfo set2Info = {};
	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setInfo.pNext = nullptr;
	setInfo.bindingCount = 1;
	setInfo.flags = 0;
	setInfo.pBindings = &objectBind;

	vkCreateDescriptorSetLayout(device, &set2Info, nullptr, &objectSetLayout);

	const size_t sceneParamBufferSize = FRAME_OVERLAP * PadUniformBufferSize(sizeof(GPUSceneData));
	sceneParameterBuffer = CreateBuffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	for (int i = 0; i < FRAME_OVERLAP; ++i) {
		const int MAX_OBJECTS = 10000;
		frames[i].objectBuffer = CreateBuffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		frames[i].cameraBuffer = CreateBuffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &globalSetLayout;

		vkAllocateDescriptorSets(device, &allocInfo, &frames[i].globalDescriptor);

		VkDescriptorBufferInfo binfo = {};
		binfo.buffer = frames[i].cameraBuffer.buffer;
		binfo.offset = 0;
		binfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = frames[i].cameraBuffer.buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo = {};
		sceneInfo.buffer = sceneParameterBuffer.buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkWriteDescriptorSet cameraWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frames[i].globalDescriptor, &cameraInfo, 0);
		VkWriteDescriptorSet sceneWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, frames[i].globalDescriptor, &sceneInfo, 1);

		VkWriteDescriptorSet setWrites[] = { cameraWrite, sceneWrite };

		vkUpdateDescriptorSets(device, 2, setWrites, 0, nullptr);
	}

	mainDeletionQueue.pushFunction([&] {
		vmaDestroyBuffer(allocator, sceneParameterBuffer.buffer, sceneParameterBuffer.allocation);
		vkDestroyDescriptorSetLayout(device, globalSetLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		for (int i = 0; i < FRAME_OVERLAP; ++i) {
			vmaDestroyBuffer(allocator, frames[i].cameraBuffer.buffer, frames[i].cameraBuffer.allocation);
		}
	});
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
	pipelineInfo.pDepthStencilState = &depthStencil;
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

void VulkanEngine::LoadMeshes() {
	Mesh triangleMesh{};
	triangleMesh.vertices.resize(3);

	triangleMesh.vertices[0].position = {  1.f,  1.f, 0.0f };
	triangleMesh.vertices[1].position = { -1.f,  1.f, 0.0f };
	triangleMesh.vertices[2].position = {  0.f, -1.f, 0.0f };

	triangleMesh.vertices[0].color = { 0.f, 1.f, 0.f };
	triangleMesh.vertices[1].color = { 0.f, 1.f, 0.f };
	triangleMesh.vertices[2].color = { 0.f, 1.f, 0.f };

	Mesh monkeyMesh{};
	monkeyMesh.LoadFromObj("../assets/monkey_smooth.obj");

	UploadMesh(triangleMesh);
	UploadMesh(monkeyMesh);

	meshes["monkey"] = monkeyMesh;
	meshes["triangle"] = triangleMesh;
}

void VulkanEngine::UploadMesh(Mesh& mesh) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &mesh.vertexBuffer.buffer, &mesh.vertexBuffer.allocation, nullptr));

	mainDeletionQueue.pushFunction([=]() {
		vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
	});

	void* data;
	vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);
	memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
	vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
}

void VulkanEngine::InitScene() {
	RenderObject monkey;
	monkey.mesh = GetMesh("monkey");
	monkey.material = GetMaterial("defaultmesh");
	monkey.transformMatrix = glm::mat4{ 1.0f };

	renderables.push_back(monkey);

	for (int x = -20; x <= 20; ++x) {
		for (int y = -20; y <= 20; ++y) {
			RenderObject tri;
			tri.mesh = GetMesh("triangle");
			tri.material = GetMaterial("defaultmesh");
			glm::mat4 translation = glm::translate(glm::mat4{ 1.0f }, glm::vec3(x, 0, y));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.2f, 0.2f, 0.2f));
			tri.transformMatrix = translation * scale;

			renderables.push_back(tri);
		}
	}
}

Material* VulkanEngine::CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name) {
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	materials[name] = mat;
	return &materials[name];
}

Material* VulkanEngine::GetMaterial(const std::string& name) {
	auto it = materials.find(name);
	if (it == materials.end())
		return nullptr;
	
	return &it->second;
}

AllocatedBuffer VulkanEngine::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaAllocInfo = {};
	vmaAllocInfo.usage = memoryUsage;

	AllocatedBuffer buffer;
	VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo, &buffer.buffer, &buffer.allocation, nullptr));

	return buffer;
}

Mesh* VulkanEngine::GetMesh(const std::string& name) {
	auto it = meshes.find(name);
	if (it == meshes.end())
		return nullptr;

	return &it->second;
}

void VulkanEngine::DrawObjects(VkCommandBuffer cmd, RenderObject* first, int count) {
	glm::vec3 camPos = { 0.f, -6.f, -10.f };
	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f);
	projection[1][1] *= -1;

	GPUCameraData camData;
	camData.projection = projection;
	camData.view = view;
	camData.viewproj = projection * view;

	void* data;
	vmaMapMemory(allocator, GetCurrentFrame().cameraBuffer.allocation, &data);
	memcpy(data, &camData, sizeof(GPUCameraData));
	vmaUnmapMemory(allocator, GetCurrentFrame().cameraBuffer.allocation);

	float framed = frameNumber / 120.f;
	sceneParameters.ambientColor = { sin(framed), 0, cos(framed), 1 };
	char* sceneData;
	vmaMapMemory(allocator, sceneParameterBuffer.allocation, (void**)&sceneData);
	int frameIndex = frameNumber % FRAME_OVERLAP;
	sceneData += PadUniformBufferSize(sizeof(GPUSceneData)) * frameIndex;
	memcpy(sceneData, &sceneParameters, sizeof(GPUSceneData));
	vmaUnmapMemory(allocator, sceneParameterBuffer.allocation);

	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	for (int i = 0; i < count; ++i) {
		RenderObject& object = first[i];
		if (object.material != lastMaterial) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
			lastMaterial = object.material;
			uint32_t uniformOffset = PadUniformBufferSize(sizeof(sceneData)) * frameIndex;
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1, &GetCurrentFrame().globalDescriptor, 1, &uniformOffset);
		}

		glm::mat4 model = object.transformMatrix;
		glm::mat4 meshMatrix = projection * view * model;

		MeshPushConstants constants;
		constants.renderMatrix = object.transformMatrix;

		vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		if (object.mesh != lastMesh) {
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer.buffer, &offset);
			lastMesh = object.mesh;
		}

		vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
	}
}

size_t VulkanEngine::PadUniformBufferSize(size_t originalSize) {
	size_t minUboAligment = gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAligment > 0)
		alignedSize = (alignedSize + minUboAligment - 1) & ~(minUboAligment - 1);
	return alignedSize;
}

void DeletionQueue::pushFunction(std::function<void()>&& function) {
	deletors.push_back(function);
}

void DeletionQueue::flush(){
	for (auto it = deletors.rbegin(); it != deletors.rend(); ++it) {
		(*it)();
	}

	deletors.clear();
}

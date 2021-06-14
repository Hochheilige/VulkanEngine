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

static const VertexPC coloredCubeData[] =
{
	// red face
	{ -1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	// green face
	{ -1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	// blue face
	{ -1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	// yellow face
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	// magenta face
	{  1.0f,  1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f,  1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	// cyan face
	{  1.0f, -1.0f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f },
	{  1.0f, -1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
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
	base->Init(rendererWindow->GetWindow());

	Swapchain swapchain(*base);

	Image depthImage(base->GetPhysicalDevice(), vk::Format::eD16Unorm);
	depthImage.Init(*base, vk::ImageAspectFlagBits::eDepth);

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

	Buffer<glm::mat4x4> uniformBuffer(base->GetPhysicalDevice(), vk::BufferUsageFlagBits::eUniformBuffer);
	uniformBuffer.Init(base->GetDevice());
	uniformBuffer.BindBuffer(base->GetDevice());

	// vertex buffer
	Buffer<VertexPC[36]> vertexBuffer(base->GetPhysicalDevice(), vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBuffer.Init(base->GetDevice());
	vertexBuffer.CopyBuffer(base->GetDevice(), coloredCubeData);

	DescriptorSet descriptorSet(
		0, vk::DescriptorType::eUniformBuffer,
		1, vk::ShaderStageFlagBits::eVertex
	);

	descriptorSet.CreateDescriptorSetAndPipelineLayouts(base->GetDevice());
	descriptorSet.CreateDescriptorPool(base->GetDevice(), vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1);
	descriptorSet.CreateDescriptorSet(base->GetDevice());
	descriptorSet.UpdateDescriptorSet<glm::mat4x4>(base->GetDevice(), uniformBuffer.GetBuffer());

	RenderPass renderPass(base->GetFormat(), depthImage.GetFormat());
	renderPass.Create(base->GetDevice());
	
	// shader modules stuff
	vk::ShaderModule cubeVertexShaderModule = loadShaderModule("../shaders/cube.vert.spv", base->GetDevice());
	vk::ShaderModule cubeFragmentShaderModule = loadShaderModule("../shaders/cube.frag.spv", base->GetDevice());

	// frame buffer stuff
	Framebuffer framebuffer(
		renderPass.GetRenderPass(),
		swapchain.GetExtent(),
		depthImage.GetImageView()
	);
	framebuffer.Create(base->GetDevice(), swapchain);
	
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

	// pipeline 
	PipelineBuilder pipelineBuilder;
	pipelineBuilder.colorBlendAttachment = utils::colorBlendAttachmentState();
	pipelineBuilder.depthStencil = utils::depthStencilCreateInfo(true, true, vk::CompareOp::eLessOrEqual);
	pipelineBuilder.shaderStages.push_back(utils::pipelineShaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex, cubeVertexShaderModule));
	pipelineBuilder.shaderStages.push_back(utils::pipelineShaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment, cubeFragmentShaderModule));
	pipelineBuilder.vertexInputInfo = utils::vertexInputStateCreateInfo();
	
	vk::VertexInputBindingDescription vertexInputBindingDescription(
		0,
		sizeof(coloredCubeData[0])
	);

	std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributeDescriptions = {
		vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, 16),
		vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, 0)
	};

	pipelineBuilder.vertexInputInfo.setVertexBindingDescriptions(vertexInputBindingDescription);
	pipelineBuilder.vertexInputInfo.setVertexAttributeDescriptions(vertexInputAttributeDescriptions);

	pipelineBuilder.inputAssembly = utils::inputAssemblyCreateInfo(vk::PrimitiveTopology::eTriangleList);
	//pipelineBuilder.viewport.setX(0.0f);
	//pipelineBuilder.viewport.setY(0.0f);
	//pipelineBuilder.viewport.setWidth(swapchain.GetExtent().width);
	//pipelineBuilder.viewport.setHeight(swapchain.GetExtent().height);
	//pipelineBuilder.viewport.setMinDepth(0.0f);
	//pipelineBuilder.viewport.setMaxDepth(1.0f);

	pipelineBuilder.pipelineLayout = descriptorSet.GetPipelineLayout();
	pipelineBuilder.rasterizer = utils::rasterizationStateCreateInfo(vk::PolygonMode::eFill);
	pipelineBuilder.multisampling = utils::multisamplingStateCreateInfo();

	std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	pipelineBuilder.dynamicStateCreateInfo = {
		vk::PipelineDynamicStateCreateFlags(),
		dynamicStates
	};

	vk::Pipeline graphicsPipeline = pipelineBuilder.Build(base->GetDevice(), renderPass.GetRenderPass());

	//vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
	//	vk::PipelineColorBlendStateCreateFlags(),  // flags
	//	false,                                     // logicOpEnable
	//	vk::LogicOp::eNoOp,                        // logicOp
	//	pipelineColorBlendAttachmentState,         // attachments
	//	{ { 1.0f, 1.0f, 1.0f, 1.0f } }             // blendConstants
	//);

	


	//vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
	//	vk::PipelineCreateFlags(),
	//	pipelineShaderStagesCreateInfos,
	//	&pipelineVertexInputStateCreateInfo,
	//	&pipelineInputAssemblyCreateInfo,
	//	nullptr,
	//	&pipelineViewportStateCreateInfo,
	//	&pipelineRasterizationStateCreateInfo,
	//	&pipelineMultisampleStateCreateInfo,
	//	&pipelineDepthStencilStateCreateInfo,
	//	&pipelineColorBlendStateCreateInfo,
	//	&pipelineDynamicStateCreateInfo,
	//	descriptorSet.GetPipelineLayout(),
	//	renderPass.GetRenderPass()
	//);

	//vk::Result result;
	//vk::Pipeline pipeline;
	//std::tie(result, pipeline) = base->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);

	std::vector<glm::vec3> cubePositions = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, 15.0f),
		glm::vec3(-1.5f, -2.2f, 2.5f),
		glm::vec3(-3.8f, -2.0f, 12.3f),
		glm::vec3(2.4f, -0.4f, 3.5f),
		glm::vec3(-1.7f,  3.0f, 7.5f),
		glm::vec3(1.3f, -2.0f, 2.5f),
		glm::vec3(1.5f,  2.0f, 2.5f),
		glm::vec3(1.5f,  0.2f, 1.5f),
		glm::vec3(-1.3f,  1.0f, 1.5f),
	};

	uniformBuffer.MapBuffer(base->GetDevice());

	uint32_t frameNumber = 0;
	while (!rendererWindow->isShouldClose) {
		while (rendererWindow->PollEvents(&event)) {
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
				rendererWindow->isShouldClose = true;
		}

		// Not sure that this is part of vertex buffer stuff
		vk::Semaphore imageAcquiredSemaphore = base->GetDevice().createSemaphore(vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags()));

		vk::ResultValue<uint32_t> currentBuffer = base->GetDevice().acquireNextImageKHR(
			swapchain.GetSwapchain(),
			1000000000,
			imageAcquiredSemaphore,
			nullptr
		);
		assert(currentBuffer.result == vk::Result::eSuccess);
		assert(currentBuffer.value < framebuffer.GetFramebuffers().size());

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


		commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

		vk::RenderPassBeginInfo renderPassBeginInfo(
			renderPass.GetRenderPass(),
			framebuffer.GetFramebuffers()[currentBuffer.value],
			vk::Rect2D(vk::Offset2D(0, 0), swapchain.GetExtent()),
			clearValues
		);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
		// ???
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, descriptorSet.GetPipelineLayout(), 0, descriptorSet.GetDescriptorSet(), nullptr);

		commandBuffer.bindVertexBuffers(0, vertexBuffer.GetBuffer(), { 0 });

		commandBuffer.setViewport(
			0,
			vk::Viewport(
				0.0f, 0.0f,
				static_cast<float>(swapchain.GetExtent().width),
				static_cast<float>(swapchain.GetExtent().height),
				0.0f,
				1.0f
			)
		);

		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.GetExtent()));

		model = glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[0]);
		model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
		model = glm::rotate(model, glm::radians(frameNumber * 0.7f), glm::vec3(1.0f, 0.3f, 0.5f));
		mvpc = clip * projection * view * model;
		
		++frameNumber;

		uniformBuffer.MemoryCopy(mvpc);

		commandBuffer.draw(36, 1, 0, 0);
		

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

		vk::Result drawResult = base->GetQueues().graphicsQueue.presentKHR(vk::PresentInfoKHR({}, swapchain.GetSwapchain(), currentBuffer.value));

		base->GetDevice().waitIdle();

		base->GetDevice().destroyFence(drawFence);
		base->GetDevice().destroySemaphore(imageAcquiredSemaphore);
	}
	
	base->GetDevice().destroyPipeline(graphicsPipeline);

	base->GetDevice().freeMemory(vertexBuffer.GetDeviceMemory());
	base->GetDevice().destroyBuffer(vertexBuffer.GetBuffer());

	base->GetDevice().freeCommandBuffers(commandPool, commandBuffer);
	base->GetDevice().destroyCommandPool(commandPool);

	for (const auto& framebuffer : framebuffer.GetFramebuffers()) {
		base->GetDevice().destroyFramebuffer(framebuffer);
	}

	base->GetDevice().destroyShaderModule(cubeFragmentShaderModule);
	base->GetDevice().destroyShaderModule(cubeVertexShaderModule);
	
	base->GetDevice().destroyRenderPass(renderPass.GetRenderPass());

	base->GetDevice().freeDescriptorSets(descriptorSet.GetDescriptorPool(), descriptorSet.GetDescriptorSet());
	base->GetDevice().destroyDescriptorPool(descriptorSet.GetDescriptorPool());

	base->GetDevice().destroyPipelineLayout(descriptorSet.GetPipelineLayout());
	base->GetDevice().destroyDescriptorSetLayout(descriptorSet.GetDescriptorSetLayout());

	base->GetDevice().freeMemory(uniformBuffer.GetDeviceMemory());
	base->GetDevice().destroyBuffer(uniformBuffer.GetBuffer());
	base->GetDevice().destroyImageView(depthImage.GetImageView());
	base->GetDevice().freeMemory(depthImage.GetDeviceMemory());
	base->GetDevice().destroyImage(depthImage.GetImage());
	for (auto& imageView : swapchain.GetImageViews()) {
		base->GetDevice().destroyImageView(imageView);
	}
	base->GetDevice().destroySwapchainKHR(swapchain.GetSwapchain());

	return 0;
}
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
#include <chrono>

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
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_CaptureMouse(SDL_TRUE);
	SDL_Event event;
	
	std::unique_ptr<VulkanBase> base = std::make_unique<VulkanBase>();
	base->Init(rendererWindow->GetWindow());

	Swapchain swapchain(*base);

	Image depthImage(base->GetPhysicalDevice(), vk::Format::eD16Unorm);
	depthImage.Init(*base, vk::ImageAspectFlagBits::eDepth);

	// Camera data
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float currentFrame = 0.0f;
	float cameraSpeed = 0.0f;

	float yaw = -90.0f;
	float pitch = 0.0f;
	int xpos = 0, ypos = 0;
	int lastX = swapchain.GetExtent().width / 2.0f;
	int lastY = swapchain.GetExtent().height / 2.0f;
	bool firstMouse = true;

	// Uniform buffer data
	glm::mat4x4 model = glm::mat4x4(1.0f);
	glm::mat4x4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4x4 projection = glm::perspective(glm::radians(45.0f), (float)swapchain.GetExtent().width / swapchain.GetExtent().height, 0.1f, 100.0f);
	glm::mat4x4 clip = glm::mat4x4(
		1.0f,  0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f,  0.0f, 0.5f, 0.0f,
		0.0f,  0.0f, 0.5f, 1.0f
	);
	glm::mat4x4 mvpc = clip * projection * view * model;

	Buffer uniformBuffer(base->GetPhysicalDevice(), vk::BufferUsageFlagBits::eUniformBuffer);
	uniformBuffer.Init<glm::mat4x4>(base->GetDevice());
	uniformBuffer.BindBuffer(base->GetDevice());

	// vertex buffer
	Buffer vertexBuffer(base->GetPhysicalDevice(), vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBuffer.Init<VertexPC[36]>(base->GetDevice());
	vertexBuffer.CopyBuffer(base->GetDevice(), coloredCubeData);

	DescriptorSet descriptorSet(
		0, vk::DescriptorType::eUniformBuffer,
		1, vk::ShaderStageFlagBits::eVertex
	);

	descriptorSet.CreateDescriptorSetAndPipelineLayouts(base->GetDevice());
	descriptorSet.CreateDescriptorPool(base->GetDevice(), vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 10);
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
	pipelineBuilder.pipelineLayout = descriptorSet.GetPipelineLayout();
	pipelineBuilder.rasterizer = utils::rasterizationStateCreateInfo(vk::PolygonMode::eFill);
	pipelineBuilder.multisampling = utils::multisamplingStateCreateInfo();

	std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	pipelineBuilder.dynamicStateCreateInfo = {
		vk::PipelineDynamicStateCreateFlags(),
		dynamicStates
	};

	vk::Pipeline graphicsPipeline = pipelineBuilder.Build(base->GetDevice(), renderPass.GetRenderPass());

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

		currentFrame = SDL_GetTicks() * 0.001f;
		//printf("Current: %f\n", currentFrame);
		deltaTime = currentFrame - lastFrame;
		//printf("Delta: %f\n", deltaTime);
		lastFrame = currentFrame;
		//printf("Last: %f\n", lastFrame);
		

		while (rendererWindow->PollEvents(&event)) {
			cameraSpeed = 2.5f * deltaTime;
			printf("speed: %f\n", cameraSpeed);
			if (event.type == SDL_QUIT)
				rendererWindow->isShouldClose = true;
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE)
					rendererWindow->isShouldClose = true;

				if (event.key.keysym.sym == SDLK_w)
					cameraPos += cameraSpeed * cameraFront;
				if (event.key.keysym.sym == SDLK_s)
					cameraPos -= cameraSpeed * cameraFront;
				if (event.key.keysym.sym == SDLK_a)
					cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
				if (event.key.keysym.sym == SDLK_d)
					cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;	
			} 

			SDL_GetMouseState(&xpos, &ypos);
			if (firstMouse) {
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}
			
			float xoffset = xpos - lastX;
			float yoffset = ypos - lastY;
			lastX = xpos;
			lastY = ypos;

			const float sensitivity = 0.1f;
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			yaw += xoffset;
			pitch -= yoffset;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			glm::vec3 direction;
			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(pitch));
			direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			cameraFront = glm::normalize(direction);
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

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		model = glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[0]);
		model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
		model = glm::rotate(model, glm::radians(frameNumber * 0.5f), glm::vec3(1.0f, 0.3f, 0.5f));
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
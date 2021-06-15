#include <Engine.hpp>

Engine::Engine() {
}

Engine::~Engine() {
}

void Engine::Init() {
	window = Window::CreateWindow("Vulkan Renderer", 1024, 768);
	vulkanBase.Init(window->GetWindow());
	swapchain.Init(vulkanBase);
	depthImage.Init(vulkanBase, vk::Format::eD16Unorm, vk::ImageAspectFlagBits::eDepth, true);
	renderPass.Create(vulkanBase.GetDevice(), vulkanBase.GetFormat(), depthImage.GetFormat());
	framebuffer.Create(vulkanBase.GetDevice(), renderPass.GetRenderPass(), swapchain, depthImage.GetImageView());
	InitCommands();
	InitSyncStructures();
	InitPipelines();
	LoadMeshes();
}

void Engine::Run() {
	SDL_Event event;
	bool isCaptureMouse = false;

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

	while (!window->isShouldClose) {

		currentFrame = SDL_GetTicks() * 0.001f;
		//printf("Current: %f\n", currentFrame);
		deltaTime = currentFrame - lastFrame;
		//printf("Delta: %f\n", deltaTime);
		lastFrame = currentFrame;
		//printf("Last: %f\n", lastFrame);


		while (window->PollEvents(&event)) {
			cameraSpeed = 2.5f * deltaTime;
			printf("speed: %f\n", cameraSpeed);
			if (event.type == SDL_QUIT)
				window->isShouldClose = true;
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE)
					window->isShouldClose = true;

				if (event.key.keysym.sym == SDLK_TAB) {
					if (isCaptureMouse) {
						SDL_SetRelativeMouseMode(SDL_FALSE);
						SDL_CaptureMouse(SDL_FALSE);
						isCaptureMouse = false;
					}
					else {
						SDL_SetRelativeMouseMode(SDL_TRUE);
						SDL_CaptureMouse(SDL_TRUE);
						isCaptureMouse = true;
					}
				}


				if (event.key.keysym.sym == SDLK_w)
					cameraPos += cameraSpeed * cameraFront;
				if (event.key.keysym.sym == SDLK_s)
					cameraPos -= cameraSpeed * cameraFront;
				if (event.key.keysym.sym == SDLK_a)
					cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
				if (event.key.keysym.sym == SDLK_d)
					cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			}

			if (isCaptureMouse) {
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
		}

		Draw();
	}
}

void Engine::CleanUp() {
	vulkanBase.GetDevice().waitIdle();

	for (uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		vulkanBase.GetDevice().destroyFence(frames[i].renderFence);
		vulkanBase.GetDevice().destroySemaphore(frames[i].renderSemaphore);
		vulkanBase.GetDevice().destroySemaphore(frames[i].presentSemaphore);
	}

	vulkanBase.GetDevice().destroyPipeline(rotatingColoredCubePipeline);
	vulkanBase.GetDevice().freeMemory(coloredCube.buffer.GetDeviceMemory());
	vulkanBase.GetDevice().destroyBuffer(coloredCube.buffer.GetBuffer());

	for (uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		vulkanBase.GetDevice().freeCommandBuffers(frames[i].commandPool, frames[i].commandBuffer);
		vulkanBase.GetDevice().destroyCommandPool(frames[i].commandPool);
	}

	for (const auto& framebuffer : framebuffer.GetFramebuffers()) {
		vulkanBase.GetDevice().destroyFramebuffer(framebuffer);
	}

	vulkanBase.GetDevice().destroyRenderPass(renderPass.GetRenderPass());

	vulkanBase.GetDevice().destroyImageView(depthImage.GetImageView());
	vulkanBase.GetDevice().freeMemory(depthImage.GetDeviceMemory());
	vulkanBase.GetDevice().destroyImage(depthImage.GetImage());
	for (auto& imageView : swapchain.GetImageViews()) {
		vulkanBase.GetDevice().destroyImageView(imageView);
	}
	vulkanBase.GetDevice().destroySwapchainKHR(swapchain.GetSwapchain());
}

void Engine::InitCommands() {
	vk::CommandPoolCreateInfo poolInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		vulkanBase.GetQueues().graphicsQueueIndex
	);

	for (uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		frames[i].commandPool = vulkanBase.GetDevice().createCommandPool(poolInfo);
		frames[i].commandBuffer = vulkanBase.GetDevice().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				frames[i].commandPool,
				vk::CommandBufferLevel::ePrimary,
				1
			)
		).front();
	}

	vk::CommandPoolCreateInfo uploadInfo({}, vulkanBase.GetQueues().graphicsQueueIndex);
	uploadContext.commandPool = vulkanBase.GetDevice().createCommandPool(uploadInfo);
}

void Engine::InitSyncStructures() {
	for (uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		frames[i].renderFence = vulkanBase.GetDevice().createFence(
			vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)
		);
		frames[i].presentSemaphore = vulkanBase.GetDevice().createSemaphore(
			vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags())
		);
		frames[i].renderSemaphore = vulkanBase.GetDevice().createSemaphore(
			vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags())
		);
	}

	uploadContext.uploadFence = vulkanBase.GetDevice().createFence(
		vk::FenceCreateInfo()
	);
}

void Engine::InitPipelines() {

	vk::ShaderModule cubeVertexShaderModule = utils::loadShaderModule("../shaders/cube.vert.spv", vulkanBase.GetDevice());
	vk::ShaderModule cubeFragmentShaderModule = utils::loadShaderModule("../shaders/cube.frag.spv", vulkanBase.GetDevice());

	rotatingColoredCubePipelineLayout = vulkanBase.GetDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags())
	);

	PipelineBuilder pipelineBuilder;

	VertexInputDescription vertexDescription = Vertex::GetVertexDescription();

	pipelineBuilder.vertexInputInfo = utils::vertexInputStateCreateInfo();
	pipelineBuilder.vertexInputInfo.setVertexAttributeDescriptions(vertexDescription.attributes);
	pipelineBuilder.vertexInputInfo.setVertexBindingDescriptions(vertexDescription.bindings);

	pipelineBuilder.shaderStages.push_back(
		utils::pipelineShaderStageCreateInfo(
			vk::ShaderStageFlagBits::eVertex,
			cubeVertexShaderModule
		)
	);
	pipelineBuilder.shaderStages.push_back(
		utils::pipelineShaderStageCreateInfo(
			vk::ShaderStageFlagBits::eFragment,
			cubeFragmentShaderModule
		)
	);

	pipelineBuilder.inputAssembly = utils::inputAssemblyCreateInfo(vk::PrimitiveTopology::eTriangleList);
	
	std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	//TODO: Добавить в ютилс
	pipelineBuilder.dynamicStateCreateInfo = {
		vk::PipelineDynamicStateCreateFlags(),
		dynamicStates
	};

	pipelineBuilder.rasterizer = utils::rasterizationStateCreateInfo(vk::PolygonMode::eFill);
	pipelineBuilder.multisampling = utils::multisamplingStateCreateInfo();
	pipelineBuilder.colorBlendAttachment = utils::colorBlendAttachmentState();
	pipelineBuilder.pipelineLayout = rotatingColoredCubePipelineLayout;

	rotatingColoredCubePipeline = pipelineBuilder.Build(vulkanBase.GetDevice(), renderPass.GetRenderPass());

	vulkanBase.GetDevice().destroyShaderModule(cubeVertexShaderModule);
	vulkanBase.GetDevice().destroyShaderModule(cubeFragmentShaderModule);
}

void Engine::Draw() {
	vulkanBase.GetDevice().waitForFences(GetCurrentFrame().renderFence, true, 1000000000);
	vulkanBase.GetDevice().resetFences(GetCurrentFrame().renderFence);
	GetCurrentFrame().commandBuffer.reset();

	uint32_t swapchainImageIndex = vulkanBase.GetDevice()
		.acquireNextImageKHR(
			swapchain.GetSwapchain(),
			1000000000,
			GetCurrentFrame().presentSemaphore
		);

	vk::CommandBuffer cmd = GetCurrentFrame().commandBuffer;

	cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

	std::array<vk::ClearValue, 2> clearValues;
	// random colors to window color
	srand(time(nullptr));
	float redFlash = abs(sin(rand() / 120.0f));
	float greenFlash = abs(sin(rand() / 120.0f));
	float blueFlash = abs(sin(rand() / 120.0f));
	float alphaFlash = abs(sin(rand() / 120.0f));
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ {redFlash, greenFlash, blueFlash, alphaFlash} }));
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderPassBeginInfo renderPassBeginInfo(
		renderPass.GetRenderPass(),
		framebuffer.GetFramebuffers()[swapchainImageIndex],
		vk::Rect2D(vk::Offset2D(0, 0), swapchain.GetExtent()),
		clearValues
	);

	cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, rotatingColoredCubePipeline);
	// ???
	//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, descriptorSet.GetPipelineLayout(), 0, descriptorSet.GetDescriptorSet(), nullptr);
	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, coloredCube.buffer.GetBuffer(), { offset });
	

	cmd.setViewport(
		0,
		vk::Viewport(
			0.0f, 0.0f,
			static_cast<float>(swapchain.GetExtent().width),
			static_cast<float>(swapchain.GetExtent().height),
			0.0f,
			1.0f
		)
	);

	cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.GetExtent()));

	cmd.draw(coloredCube.vertices.size(), 1, 0, 0);

	cmd.endRenderPass();
	cmd.end();

	vk::PipelineStageFlags waitStage(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	vk::SubmitInfo submitInfo(
		GetCurrentFrame().presentSemaphore,
		waitStage, 
		cmd,
		GetCurrentFrame().renderSemaphore
	);

	vulkanBase.GetQueues().graphicsQueue.submit(submitInfo, GetCurrentFrame().renderFence);

	vulkanBase.GetQueues().graphicsQueue.presentKHR(
		vk::PresentInfoKHR(
			GetCurrentFrame().renderSemaphore,
			swapchain.GetSwapchain(),
			swapchainImageIndex
		)
	);

	++frameNumber;
}

void Engine::LoadMeshes() {
	coloredCube.vertices.resize(36); {
		coloredCube.vertices[0].position = { -1.0f, -1.0f, 1.0f };
		coloredCube.vertices[0].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[1].position = { -1.0f,  1.0f,  1.0f };
		coloredCube.vertices[1].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[2].position = { 1.0f, -1.0f,  1.0f };
		coloredCube.vertices[2].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[3].position = { 1.0f, -1.0f,  1.0f };
		coloredCube.vertices[3].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[4].position = { -1.0f,  1.0f,  1.0f };
		coloredCube.vertices[4].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[5].position = { 1.0f,  1.0f,  1.0f };
		coloredCube.vertices[5].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[6].position = { -1.0f, -1.0f, -1.0f };
		coloredCube.vertices[6].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[7].position = { 1.0f, -1.0f, -1.0f };
		coloredCube.vertices[7].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[8].position = { -1.0f,  1.0f, -1.0f };
		coloredCube.vertices[8].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[9].position = { -1.0f,  1.0f, -1.0f };
		coloredCube.vertices[9].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[10].position = { 1.0f, -1.0f, -1.0f };
		coloredCube.vertices[10].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[11].position = { 1.0f,  1.0f, -1.0f };
		coloredCube.vertices[11].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[12].position = { -1.0f,  1.0f,  1.0f };
		coloredCube.vertices[12].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[13].position = { -1.0f, -1.0f,  1.0f };
		coloredCube.vertices[13].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[14].position = { -1.0f,  1.0f, -1.0f };
		coloredCube.vertices[14].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[15].position = { -1.0f,  1.0f, -1.0f };
		coloredCube.vertices[15].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[16].position = { -1.0f, -1.0f,  1.0f };
		coloredCube.vertices[16].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[17].position = { -1.0f, -1.0f, -1.0f };
		coloredCube.vertices[17].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[18].position = { 1.0f,  1.0f,  1.0f };
		coloredCube.vertices[18].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[19].position = { 1.0f,  1.0f, -1.0f };
		coloredCube.vertices[19].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[20].position = { 1.0f, -1.0f,  1.0f };
		coloredCube.vertices[20].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[21].position = { 1.0f, -1.0f,  1.0f };
		coloredCube.vertices[21].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[22].position = { 1.0f,  1.0f, -1.0f };
		coloredCube.vertices[22].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[23].position = { 1.0f, -1.0f, -1.0f };
		coloredCube.vertices[23].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[24].position = { 1.0f,  1.0f,  1.0f };
		coloredCube.vertices[24].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[25].position = { -1.0f,  1.0f,  1.0f };
		coloredCube.vertices[25].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[26].position = { 1.0f,  1.0f, -1.0f };
		coloredCube.vertices[26].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[27].position = { 1.0f,  1.0f, -1.0f };
		coloredCube.vertices[27].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[28].position = { -1.0f,  1.0f,  1.0f };
		coloredCube.vertices[28].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[29].position = { -1.0f,  1.0f, -1.0f };
		coloredCube.vertices[29].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[30].position = { 1.0f, -1.0f,  1.0f };
		coloredCube.vertices[30].color = { 1.0f, 0.0f, 0.0f };
		coloredCube.vertices[31].position = { 1.0f, -1.0f, -1.0f };
		coloredCube.vertices[31].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[32].position = { -1.0f, -1.0f,  1.0f };
		coloredCube.vertices[32].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[33].position = { -1.0f, -1.0f,  1.0f };
		coloredCube.vertices[33].color = { 0.0f, 0.0f, 1.0f };
		coloredCube.vertices[34].position = { 1.0f, -1.0f, -1.0f };
		coloredCube.vertices[34].color = { 0.0f, 1.0f, 0.0f };
		coloredCube.vertices[35].position = { -1.0f, -1.0f, -1.0f };
		coloredCube.vertices[35].color = { 1.0f, 0.0f, 0.0f };
	}
	UploadMeshes(coloredCube);

	//_triangleMesh.vertices.resize(3);

	////vertex positions
	//_triangleMesh.vertices[0].position = { 1.f, 1.f, 0.0f };
	//_triangleMesh.vertices[1].position = { -1.f, 1.f, 0.0f };
	//_triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

	////vertex colors, all green
	//_triangleMesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	//_triangleMesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	//_triangleMesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

	////we don't care about the vertex normals

	//UploadMeshes(_triangleMesh);

}

void Engine::UploadMeshes(Mesh& mesh) {

	mesh.buffer.Init<Vertex>(vulkanBase.GetPhysicalDevice(), vulkanBase.GetDevice(), 
		vk::BufferUsageFlagBits::eVertexBuffer, mesh.vertices.size());
	mesh.buffer.CopyBuffer(vulkanBase.GetDevice(), mesh.vertices);

}


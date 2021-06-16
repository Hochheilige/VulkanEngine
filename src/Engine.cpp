#include <Engine.hpp>

Engine::Engine() {
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

Engine::~Engine() {
}

void Engine::Init() {
	window = Window::CreateWindow("Vulkan Renderer", 1024, 768);
	vulkanBase.Init(window->GetWindow());
	swapchain.Init(vulkanBase);
	depthImage.Init(vulkanBase, vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth, true);
	renderPass.Create(vulkanBase.GetDevice(), vulkanBase.GetFormat(), depthImage.GetFormat());
	framebuffer.Create(vulkanBase.GetDevice(), renderPass.GetRenderPass(), swapchain, depthImage.GetImageView());
	InitCommands();
	InitSyncStructures();
	InitPipelines();
	LoadMeshes();
	InitScene();
}

void Engine::Run() {
	SDL_Event event;
	bool isCaptureMouse = false;

	// Camera data

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
		vulkanBase.GetDevice().freeCommandBuffers(frames[i].commandPool, frames[i].commandBuffer);
		vulkanBase.GetDevice().destroyCommandPool(frames[i].commandPool);
	}

	vulkanBase.GetDevice().destroyFence(uploadContext.uploadFence);
	vulkanBase.GetDevice().destroyCommandPool(uploadContext.commandPool);

	vulkanBase.GetDevice().destroyPipelineLayout(meshPipelineLayout);
	vulkanBase.GetDevice().destroyPipeline(meshPipeline);

	vulkanBase.GetDevice().freeMemory(monkey.buffer.GetDeviceMemory());
	vulkanBase.GetDevice().destroyBuffer(monkey.buffer.GetBuffer());
	vulkanBase.GetDevice().freeMemory(_triangleMesh.buffer.GetDeviceMemory());
	vulkanBase.GetDevice().destroyBuffer(_triangleMesh.buffer.GetBuffer());

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

	vk::PushConstantRange pushConstant(
		vk::ShaderStageFlagBits::eVertex,
		0,
		sizeof(MeshPushConstant)
	);

	meshPipelineLayout = vulkanBase.GetDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			{},
			pushConstant
		)
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

	pipelineBuilder.depthStencil = utils::depthStencilCreateInfo(true, true, vk::CompareOp::eLessOrEqual);
	pipelineBuilder.inputAssembly = utils::inputAssemblyCreateInfo(vk::PrimitiveTopology::eTriangleList);
	
	std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	//TODO: �������� � �����
	pipelineBuilder.dynamicStateCreateInfo = {
		vk::PipelineDynamicStateCreateFlags(),
		dynamicStates
	};

	pipelineBuilder.rasterizer = utils::rasterizationStateCreateInfo(vk::PolygonMode::eFill);
	pipelineBuilder.multisampling = utils::multisamplingStateCreateInfo();
	pipelineBuilder.colorBlendAttachment = utils::colorBlendAttachmentState();
	pipelineBuilder.pipelineLayout = meshPipelineLayout;

	meshPipeline = pipelineBuilder.Build(vulkanBase.GetDevice(), renderPass.GetRenderPass());

	CreateMaterial(meshPipeline, meshPipelineLayout, "defaultMesh");

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
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ {0.2f, 0.2f, 0.2f, 1} }));
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderPassBeginInfo renderPassBeginInfo(
		renderPass.GetRenderPass(),
		framebuffer.GetFramebuffers()[swapchainImageIndex],
		vk::Rect2D(vk::Offset2D(0, 0), swapchain.GetExtent()),
		clearValues
	);

	cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	//cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, rotatingColoredCubePipeline);
	// ???
	//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, descriptorSet.GetPipelineLayout(), 0, descriptorSet.GetDescriptorSet(), nullptr);
	/*vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, monkey.buffer.GetBuffer(), { offset });*/
	
	cmd.setViewport(
		0,
		vk::Viewport(
			0.0f,
			0.0f, //static_cast<float>(swapchain.GetExtent().height),
			static_cast<float>(swapchain.GetExtent().width),
			static_cast<float>(swapchain.GetExtent().height),//-static_cast<float>(swapchain.GetExtent().height),
			0.0f,
			1.0f
		)
	);

	cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.GetExtent()));

	/*glm::mat4x4 model = glm::mat4x4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
	model = glm::rotate(model, glm::radians(frameNumber * 0.5f), glm::vec3(1.0f, 0.3f, 0.5f));
	glm::mat4x4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4x4 projection = glm::perspective(glm::radians(45.0f), (float)swapchain.GetExtent().width / swapchain.GetExtent().height, 0.1f, 100.0f);
	glm::mat4x4 clip = glm::mat4x4(
		1.0f,  0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f,  0.0f, 0.5f, 0.0f,
		0.0f,  0.0f, 0.5f, 1.0f
	);

	MeshPushConstant pushMatrices{
		model,
		view,
		projection,
		clip
	};

	cmd.pushConstants(rotatingColoredCubePipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MeshPushConstant), &pushMatrices);*/

	//cmd.draw(monkey.vertices.size(), 1, 0, 0);

	DrawObjects(cmd, renderables);

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

	Mesh sponza;
	sponza.LoadFromObj("../assets/sponza.obj");

	UploadMeshes(sponza);

	monkey.LoadFromObj("../assets/monkey_smooth.obj");

	/*UploadMeshes(monkey);*/

	_triangleMesh.vertices.resize(3);

	//vertex positions
	_triangleMesh.vertices[0].position = { 1.f, 1.f, 0.0f };
	_triangleMesh.vertices[1].position = { -1.f, 1.f, 0.0f };
	_triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

	//vertex colors, all green
	_triangleMesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	_triangleMesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	_triangleMesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

	//we don't care about the vertex normals

	//UploadMeshes(_triangleMesh);

	//meshes["monkey"] = monkey;
	//meshes["triangle"] = _triangleMesh;
	meshes["sponza"] = sponza;
}

void Engine::UploadMeshes(Mesh& mesh) {

	mesh.buffer.Init<Vertex>(vulkanBase.GetPhysicalDevice(), vulkanBase.GetDevice(), 
		vk::BufferUsageFlagBits::eVertexBuffer, mesh.vertices.size());
	mesh.buffer.CopyBuffer(vulkanBase.GetDevice(), mesh.vertices);

}

void Engine::InitScene() {
	glm::mat4x4 model;
	glm::mat4x4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4x4 projection = glm::perspective(glm::radians(45.0f), (float)swapchain.GetExtent().width / swapchain.GetExtent().height, 0.1f, 10000.0f);
	glm::mat4x4 clip = glm::mat4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f
	);
	//
	//RenderObject monkey{
	//	GetMesh("monkey"),
	//	GetMaterial("defaultMesh")
	//};

	//for (int32_t x = -5; x <= 5; ++x) {
	//	for (int32_t y = -5; y <= 5; ++y) {
	//		for (int32_t z = -5; z <= 5; ++z) {
	//			model = glm::mat4x4(1.0f);
	//			model = glm::translate(model, glm::vec3(x, y, z));
	//			model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
	//			monkey.transform = {
	//				model,
	//				view,
	//				projection,
	//				clip
	//			};
	//			renderables.push_back(monkey);
	//		}
	//	}
	//}

	model = glm::mat4x4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, 0));
	model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
	//model = glm::rotate(model, glm::radians(frameNumber * 0.5f), glm::vec3(1.0f, 0.3f, 0.5f));

	//monkey.mesh = GetMesh("triangle");
	//monkey.transform = {
	//	model,
	//	view,
	//	projection,
	//	clip
	//};

	RenderObject sponza{
		GetMesh("sponza"),
		GetMaterial("defaultMesh"),
		{
			model,
			view,
			projection,
			clip
		}
	};

	renderables.push_back(sponza);
}

void Engine::DrawObjects(const vk::CommandBuffer& cmd, std::vector<RenderObject>& objects) {
	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	for (auto& object : objects) {
		if (object.material != lastMaterial) {
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, object.material->pipeline);
			lastMaterial = object.material;
		}

		object.transform.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		//object.transform.model = glm::rotate(object.transform.model, glm::radians((float)(frameNumber % 10)), glm::vec3(1.0f, 0.3f, 0.5f));

		cmd.pushConstants(object.material->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MeshPushConstant), &object.transform);

		if (object.mesh != lastMesh) {
			vk::DeviceSize offset = 0;
			cmd.bindVertexBuffers(0, 1, &object.mesh->buffer.GetBuffer(), &offset);
			lastMesh = object.mesh;
		}

		cmd.draw(object.mesh->vertices.size(), 1, 0, 0);
	}
}

const Material* Engine::CreateMaterial(const vk::Pipeline& pipeline,
	const vk::PipelineLayout& pipelineLayout, const std::string& name) {
	Material mat{
		pipeline,
		pipelineLayout,
	};
	materials[name] = mat;
	return &materials.at(name);
}

Material* Engine::GetMaterial(const std::string& name) {
	const auto iter = materials.find(name);
	if (iter == materials.end())
		return nullptr;
	return &(*iter).second;
}

Mesh* Engine::GetMesh(const std::string& name) {
	const auto iter = meshes.find(name);
	if (iter == meshes.end())
		return nullptr;
	return &(*iter).second;
}


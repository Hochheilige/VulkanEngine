#include <Engine.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Engine::Engine() {
	cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, -1.0f, 0.0f);
	camera = {
		glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp),
		glm::perspective(glm::radians(45.0f), 1024.0f / 768, 0.1f, 10000.0f),
		glm::mat4x4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f
		)
	};
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
	InitDescriptors();
	InitPipelines();
	LoadImages();
	LoadMeshes();
	InitScene();
	//InitImGui();
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
			//ImGui_ImplSDL2_ProcessEvent(&event);
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

				yaw -= xoffset;
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
		//ImGui_ImplVulkan_NewFrame();
		//ImGui_ImplSDL2_NewFrame(window->GetWindow());
		//ImGui::NewFrame();
		//ImGui::ShowDemoWindow();

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
		vulkanBase.GetDevice().freeMemory(frames[i].cameraBuffer.GetDeviceMemory());
		vulkanBase.GetDevice().destroyBuffer(frames[i].cameraBuffer.GetBuffer());
		vulkanBase.GetDevice().freeDescriptorSets(descriptorPool, frames[i].globalDescriptor);
	}

	vulkanBase.GetDevice().destroyDescriptorPool(imguiPool);
	ImGui_ImplVulkan_Shutdown();

	vulkanBase.GetDevice().destroyDescriptorSetLayout(globalSetLayout);
	vulkanBase.GetDevice().destroyDescriptorPool(descriptorPool);

	vulkanBase.GetDevice().destroyFence(uploadContext.uploadFence);
	vulkanBase.GetDevice().destroyCommandPool(uploadContext.commandPool);

	for (const auto& meshBuffer : meshBuffers) {
		vulkanBase.GetDevice().freeMemory(meshBuffer.buffer.GetDeviceMemory());
		vulkanBase.GetDevice().destroyBuffer(meshBuffer.buffer.GetBuffer());
	}

	vulkanBase.GetDevice().destroyPipelineLayout(meshPipelineLayout);
	vulkanBase.GetDevice().destroyPipeline(meshPipeline);

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

	vk::ShaderModule vertexShaderModule = utils::loadShaderModule("../shaders/cube.vert.spv", vulkanBase.GetDevice());
	vk::ShaderModule fragmentShaderModule = utils::loadShaderModule("../shaders/cube.frag.spv", vulkanBase.GetDevice());

	vk::PushConstantRange pushConstant(
		vk::ShaderStageFlagBits::eVertex,
		0,
		sizeof(MeshPushConstant)
	);

	meshPipelineLayout = vulkanBase.GetDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			globalSetLayout,
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
			vertexShaderModule
		)
	);
	pipelineBuilder.shaderStages.push_back(
		utils::pipelineShaderStageCreateInfo(
			vk::ShaderStageFlagBits::eFragment,
			fragmentShaderModule
		)
	);

	pipelineBuilder.depthStencil = utils::depthStencilCreateInfo(true, true, vk::CompareOp::eLessOrEqual);
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
	pipelineBuilder.pipelineLayout = meshPipelineLayout;

	meshPipeline = pipelineBuilder.Build(vulkanBase.GetDevice(), renderPass.GetRenderPass());

	CreateMaterial(meshPipeline, meshPipelineLayout, "defaultMesh");

	vulkanBase.GetDevice().destroyShaderModule(vertexShaderModule);
	vulkanBase.GetDevice().destroyShaderModule(fragmentShaderModule);
}

void Engine::Draw() {
	//ImGui::Render();
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
	// ???
	//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, descriptorSet.GetPipelineLayout(), 0, descriptorSet.GetDescriptorSet(), nullptr);
	
	cmd.setViewport(
		0,
		vk::Viewport(
			0.0f,
			static_cast<float>(swapchain.GetExtent().height),
			static_cast<float>(swapchain.GetExtent().width),
			-static_cast<float>(swapchain.GetExtent().height),
			0.0f,
			1.0f
		)
	);

	cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.GetExtent()));

	DrawObjects(cmd, renderables);

	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
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
	meshes["sponza"] = sponza;
	meshBuffers.push_back(sponza);

}

void Engine::UploadMeshes(Mesh& mesh) {

	mesh.buffer.Init<Vertex>(vulkanBase.GetPhysicalDevice(), vulkanBase.GetDevice(), 
		vk::BufferUsageFlagBits::eVertexBuffer, mesh.vertices.size());
	mesh.buffer.CopyBuffer(vulkanBase.GetDevice(), mesh.vertices);

}

void Engine::InitScene() {
	camera.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	camera.projection = glm::perspective(glm::radians(45.0f), (float)swapchain.GetExtent().width / swapchain.GetExtent().height, 0.1f, 10000.0f);

	RenderObject sponza{
		GetMesh("sponza"),
		GetMaterial("defaultMesh"),
		glm::translate(glm::scale(glm::mat4{1.0f}, glm::vec3(0.001, 0.001, 0.001)), glm::vec3(0, 0, 0))
	};

	renderables.push_back(sponza);
}

void Engine::DrawObjects(const vk::CommandBuffer& cmd, std::vector<RenderObject>& objects) {
	float framed = frameNumber / 60.0f;
	sceneParameters.ambientColor = { 0.3f, 0.3f, 0.3f, 1 };
	sceneParametersBuffer.MapBuffer(vulkanBase.GetDevice());
	int32_t frameIndex = frameNumber % FRAME_OVERLAP;
	*sceneParametersBuffer.GetData() += PadUnifopmBufferSize(sizeof(Scene)) * frameIndex;
	sceneParametersBuffer.MemoryCopy(sceneParameters);
	sceneParametersBuffer.UnMapBuffer(vulkanBase.GetDevice());

	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	camera.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	GetCurrentFrame().cameraBuffer.CopyBuffer(vulkanBase.GetDevice(), camera);
	for (auto& object : objects) {
		if (object.material != lastMaterial) {
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, object.material->pipeline);
			lastMaterial = object.material;

			uint32_t uniformOffset = PadUnifopmBufferSize(sizeof(Scene)) * frameIndex;

			cmd.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				object.material->pipelineLayout,
				0,
				GetCurrentFrame().globalDescriptor,
				uniformOffset
			);
		}

		cmd.pushConstants(object.material->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4x4), &object.model);

		if (object.mesh != lastMesh) {
			vk::DeviceSize offset = 0;
			cmd.bindVertexBuffers(0, 1, &object.mesh->buffer.GetBuffer(), &offset);
			lastMesh = object.mesh;
		}

		cmd.draw(object.mesh->vertices.size(), 1, 0, 0);
	}
}

void Engine::InitDescriptors() {

	std::vector<vk::DescriptorPoolSize> sizes{
		{vk::DescriptorType::eUniformBuffer, 10}, 
		{vk::DescriptorType::eUniformBufferDynamic, 10}
	};

	descriptorPool = vulkanBase.GetDevice().createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			10,
			sizes
		)
	);

	vk::DescriptorSetLayoutBinding cameraBind = utils::descriptorsetLayoutBinding(vk::DescriptorType::eUniformBuffer,
		vk::ShaderStageFlagBits::eVertex, 0);

	vk::DescriptorSetLayoutBinding sceneBind = utils::descriptorsetLayoutBinding(vk::DescriptorType::eUniformBufferDynamic,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 1);

	std::vector<vk::DescriptorSetLayoutBinding> bindings{
		cameraBind,
		sceneBind
	};

	globalSetLayout = vulkanBase.GetDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(),
			bindings
		)
	);

	size_t sceneParametersBufferSize = FRAME_OVERLAP * PadUnifopmBufferSize(sizeof(Scene));
	sceneParametersBuffer.Init<Scene>(vulkanBase.GetPhysicalDevice(), vulkanBase.GetDevice(),
		vk::BufferUsageFlagBits::eUniformBuffer, sceneParametersBufferSize);
	sceneParametersBuffer.BindBuffer(vulkanBase.GetDevice());

	for (uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		frames[i].cameraBuffer.Init<Camera>(vulkanBase.GetPhysicalDevice(), vulkanBase.GetDevice(),
			vk::BufferUsageFlagBits::eUniformBuffer);
		frames[i].cameraBuffer.BindBuffer(vulkanBase.GetDevice());

		frames[i].globalDescriptor = vulkanBase.GetDevice().allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool,
				globalSetLayout
			)
		).front();

		vk::DescriptorBufferInfo cameraInfo(
			frames[i].cameraBuffer.GetBuffer(),
			0,
			sizeof(Camera)
		);

		vk::DescriptorBufferInfo sceneInfo(
			sceneParametersBuffer.GetBuffer(),
			PadUnifopmBufferSize(sizeof(Scene)) * i,
			sizeof(Scene)
		);

		vk::WriteDescriptorSet cameraWrite = utils::writeDescriptorSet(vk::DescriptorType::eUniformBuffer, 
			frames[i].globalDescriptor, cameraInfo, 0);
		vk::WriteDescriptorSet sceneWrite = utils::writeDescriptorSet(vk::DescriptorType::eUniformBufferDynamic,
			frames[i].globalDescriptor, sceneInfo, 1);

		std::vector<vk::WriteDescriptorSet> setWrites{
			cameraWrite,
			sceneWrite
		};

		vulkanBase.GetDevice().updateDescriptorSets(setWrites, {});
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

void Engine::InitImGui() {
	std::vector<vk::DescriptorPoolSize> sizes{
		{vk::DescriptorType::eSampler, 1000},
		{vk::DescriptorType::eCombinedImageSampler, 1000},
		{vk::DescriptorType::eSampledImage, 1000},
		{vk::DescriptorType::eStorageImage, 1000},
		{vk::DescriptorType::eUniformTexelBuffer, 1000},
		{vk::DescriptorType::eStorageTexelBuffer, 1000},
		{vk::DescriptorType::eUniformBuffer, 1000},
		{vk::DescriptorType::eStorageBuffer, 1000},
		{vk::DescriptorType::eUniformBufferDynamic, 1000},
		{vk::DescriptorType::eStorageBufferDynamic, 1000},
		{vk::DescriptorType::eInputAttachment, 1000}
	};

	imguiPool = vulkanBase.GetDevice().createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			1000,
			sizes
		)
	);

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForVulkan(window->GetWindow());
	ImGui_ImplVulkan_InitInfo initInfo = {
		vulkanBase.GetInstance(),
		vulkanBase.GetPhysicalDevice(),
		vulkanBase.GetDevice(),
		vulkanBase.GetQueues().graphicsQueueIndex,
		vulkanBase.GetQueues().graphicsQueue,
		{},
		imguiPool,
		{},
		3, 3,
		(VkSampleCountFlagBits)vk::SampleCountFlagBits::e1
	};

	ImGui_ImplVulkan_Init(&initInfo, renderPass.GetRenderPass());
	
	ImmediateSubmit([&](vk::CommandBuffer cmd) {
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
	});


	ImGui_ImplVulkan_DestroyFontUploadObjects();
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

bool Engine::LoadImageFromFile(const char* file, Image& image) {
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		printf("Failed to load texture file %s\n", file);
		return false;
	}

	void* pixel_ptr = pixels;
	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	vk::Format image_format = vk::Format::eR8G8B8A8Srgb;

	Buffer stagingBuffer;
	stagingBuffer.Init<void*>(vulkanBase.GetPhysicalDevice(), vulkanBase.GetDevice(), vk::BufferUsageFlagBits::eTransferSrc, imageSize);
	stagingBuffer.CopyBuffer(vulkanBase.GetDevice(), pixel_ptr);
	stagingBuffer.BindBuffer(vulkanBase.GetDevice());

	stbi_image_free(pixels);

	vk::Extent3D imageExtent{
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight),
		1
	};

	Image newImage(imageExtent);
	vk::ImageUsageFlags flags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
	newImage.SetFlags(flags);
	newImage.Init(vulkanBase, image_format);


	//transition image to transfer-receiver	
	ImmediateSubmit([&](vk::CommandBuffer cmd) {
		vk::ImageSubresourceRange range;
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vk::ImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.oldLayout = vk::ImageLayout::eUndefined;
		imageBarrier_toTransfer.newLayout = vk::ImageLayout::eTransferDstOptimal;
		imageBarrier_toTransfer.image = newImage.GetImage();
		imageBarrier_toTransfer.subresourceRange = range;
		imageBarrier_toTransfer.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		//barrier the image into the transfer-receive layout
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, imageBarrier_toTransfer);

		vk::BufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = imageExtent;

		//copy the buffer into the image
		cmd.copyBufferToImage(stagingBuffer.GetBuffer(), newImage.GetImage(), vk::ImageLayout::eTransferDstOptimal, copyRegion);
		
		vk::ImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

		imageBarrier_toReadable.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imageBarrier_toReadable.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		imageBarrier_toReadable.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageBarrier_toReadable.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		//barrier the image into the shader readable layout
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imageBarrier_toReadable);
	});

	printf("Texture loaded succesfully %s\n",file);

	image = newImage;
	return true;
}

void Engine::LoadImages() {
	Texture sponza;

	LoadImageFromFile("../assets/lion.tga", sponza.image);

	loadedTextures["sponzaLion"] = sponza;
}

void Engine::ImmediateSubmit(std::function<void(vk::CommandBuffer cmd)>&& function) {
	vk::CommandBuffer cmd = vulkanBase.GetDevice().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(
			uploadContext.commandPool,
			{},
			1
		)
	).front();

	cmd.begin(
		vk::CommandBufferBeginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		)
	);

	function(cmd);

	cmd.end();

	vk::SubmitInfo submitInfo(
		{},
		{},
		cmd,
		{}
	);

	vulkanBase.GetQueues().graphicsQueue.submit(submitInfo, uploadContext.uploadFence);

	vulkanBase.GetDevice().waitForFences(uploadContext.uploadFence, true, 9999999999);
	vulkanBase.GetDevice().resetFences(uploadContext.uploadFence);

	vulkanBase.GetDevice().resetCommandPool(uploadContext.commandPool);

}

size_t Engine::PadUnifopmBufferSize(size_t originalSize) {
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = vulkanBase.GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}


#pragma once

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <Window.hpp>
#include <Swapchain.hpp>
#include <Image.hpp>
#include <Buffer.hpp>
#include <DescriptorSet.hpp>
#include <RenderPass.hpp>
#include <Framebuffer.hpp>
#include <PipelineBuilder.hpp>
#include <FrameData.hpp>
#include <Mesh.hpp>
#include <utils.hpp>

constexpr uint32_t FRAME_OVERLAP = 3;

struct UploadContext {
	vk::Fence uploadFence;
	vk::CommandPool commandPool;
};

class Engine {
public:
	Engine();

	~Engine();

	void Init();

	void Run();

	void CleanUp();
private:
	Window* window = nullptr;
	VulkanBase vulkanBase;
	Swapchain swapchain;
	RenderPass renderPass;
	FrameData frames[FRAME_OVERLAP];
	Framebuffer framebuffer;
	Image depthImage;

	vk::DescriptorPool descriptorPool;

	vk::PipelineLayout rotatingColoredCubePipelineLayout;
	vk::Pipeline rotatingColoredCubePipeline;
	Mesh coloredCube;
	Mesh _triangleMesh;

	UploadContext uploadContext;

	uint32_t frameNumber{ 0 };

	void InitCommands();
	void InitSyncStructures();
	void InitPipelines();

	void Draw();

	void LoadMeshes();
	void UploadMeshes(Mesh& mesh);

	const FrameData& GetCurrentFrame() const { return frames[frameNumber % FRAME_OVERLAP]; }
};
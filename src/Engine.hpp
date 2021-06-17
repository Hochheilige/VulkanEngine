#pragma once

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <unordered_map>

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
#include <RenderObject.hpp>
#include <Camera.hpp>
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
	Framebuffer framebuffer;
	Image depthImage;

	vk::DescriptorSetLayout globalSetLayout;
	vk::DescriptorPool descriptorPool;

	vk::PipelineLayout meshPipelineLayout;
	vk::Pipeline meshPipeline;
	std::vector<Mesh> meshBuffers;

	UploadContext uploadContext;

	uint32_t frameNumber{ 0 };
	uint32_t sceneNumber{ 0 };

	std::array<FrameData, FRAME_OVERLAP> frames;
	std::vector<RenderObject> renderables;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	Camera camera;

	glm::mat4x4 model;

	void InitCommands();
	void InitSyncStructures();
	void InitPipelines();
	void InitScene();
	void InitDescriptors();

	void Draw();
	void DrawObjects(const vk::CommandBuffer& cmd, std::vector<RenderObject>& objects);

	void LoadMeshes();
	void UploadMeshes(Mesh& mesh);

	FrameData& GetCurrentFrame() { return frames.at(frameNumber % FRAME_OVERLAP); }
	const Material* CreateMaterial(const vk::Pipeline& pipeline, const vk::PipelineLayout& pipelineLayout, const std::string& name);
	Material* GetMaterial(const std::string& name);
	Mesh* GetMesh(const std::string& name);
};
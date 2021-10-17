#pragma once

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_sdl.h>

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
#include <Scene.hpp>
#include <Texture.hpp>
#include <utils.hpp>

constexpr uint32_t FRAME_OVERLAP = 1;

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
	vk::DescriptorPool imguiPool;

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
	std::unordered_map<std::string, Texture> loadedTextures;

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	Camera camera;

	float ambient = 0.0f;

	Scene sceneParameters;
	Buffer sceneParametersBuffer;


	void InitCommands();
	void InitSyncStructures();
	void InitPipelines();
	void InitScene();
	void InitDescriptors();
	void InitImGui();

	void Draw();
	void DrawObjects(const vk::CommandBuffer& cmd, std::vector<RenderObject>& objects);

	void LoadMeshes();
	void UploadMeshes(Mesh& mesh);

	bool LoadImageFromFile(const char* file, Image& image);
	void LoadImages();

	FrameData& GetCurrentFrame() { return frames.at(frameNumber % FRAME_OVERLAP); }
	const Material* CreateMaterial(const vk::Pipeline& pipeline, const vk::PipelineLayout& pipelineLayout, const std::string& name);
	Material* GetMaterial(const std::string& name);
	Mesh* GetMesh(const std::string& name);
	size_t PadUnifopmBufferSize(size_t originalSize);
	void ImmediateSubmit(std::function<void(vk::CommandBuffer cmd)>&& function);
};
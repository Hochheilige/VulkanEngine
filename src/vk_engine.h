// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vk_mesh.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <deque>
#include <unordered_map>
#include <functional>

class PipelineBuilder {
public:

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState colorBlendingAttachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineLayout pipelinaLayout;

	VkPipeline Build(VkDevice device, VkRenderPass pass);

private:


};

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void pushFunction(std::function<void()>&& function);

	void flush();
};

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 renderMatrix;
};

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};

struct GPUCameraData {
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewproj;
};

struct GPUSceneData {
	glm::vec4 fogColor;
	glm::vec4 fogDistances;
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirectories;
	glm::vec4 sunlightColor;
};

struct GPUObjectData {
	glm::mat4 modelMatrix;
};

struct FrameData {
	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	AllocatedBuffer cameraBuffer;
	AllocatedBuffer objectBuffer;

	VkDescriptorSet globalDescriptor;
	VkDescriptorSet objectSetLayout;
};

constexpr uint32_t FRAME_OVERLAP = 3;

class VulkanEngine {
public:

	VulkanEngine();

	bool isInitialized;
	int frameNumber;
	int selectedShader;

	VkExtent2D windowExtent;
	struct SDL_Window* window;
	
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice gpu;
	VkPhysicalDeviceProperties gpuProperties;
	VkDevice device;

	std::vector<FrameData> frames;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	VkRenderPass renderPass;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;

	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	DeletionQueue mainDeletionQueue;

	VmaAllocator allocator;

	VkImageView depthImageView;
	AllocatedImage depthImage;
	VkFormat depthFormat;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout globalSetLayout;
	VkDescriptorSetLayout objectSetLayout;

	GPUSceneData sceneParameters;
	AllocatedBuffer sceneParameterBuffer;

	std::vector<RenderObject> renderables;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();

	FrameData& GetCurrentFrame();

private:

	void InitVulkan();

	void InitSwapchain();

	void InitCommands();

	void InitDefaultRenderpass();

	void InitFramebuffers();

	void InitSyncStructures();

	void InitDescriptors();

	void InitPipelines();

	void InitScene();

	bool LoadShaderModule(const char* filePath, VkShaderModule* outShaderModule);

	void LoadMeshes();

	void UploadMesh(Mesh& mesh);

	Material* CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	Material* GetMaterial(const std::string& name);

	Mesh* GetMesh(const std::string& name);

	void DrawObjects(VkCommandBuffer cmd, RenderObject* first, int count);

	AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	size_t PadUniformBufferSize(size_t originalSize);
};





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

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};

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

	void pushFunction(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); ++it) {
			(*it)();
		}

		deletors.clear();
	}
};

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 renderMatrix;
};

class VulkanEngine {
public:

	bool isInitialized{ false };
	int frameNumber{ 0 };
	int selectedShader{ 0 };

	VkExtent2D windowExtent{ 800, 600 };
	struct SDL_Window* window{ nullptr };
	
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice gpu;
	VkDevice device;

	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;

	VkImageView depthImageView;
	AllocatedImage depthImage;
	VkFormat depthFormat;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	VkRenderPass renderPass;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;

	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	VmaAllocator allocator;

	VkImageView depthImageView;
	AllocatedImage depthImage;
	VkFormat depthFormat;

	std::vector<RenderObject> renderables;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;

	VkPipelineLayout trianglePipelineLayout;
	VkPipelineLayout meshPipelineLayout;

	VkPipeline trianglePipeline;
	VkPipeline redTrianglePipeline;
	VkPipeline meshPipeline;

	Mesh triangleMesh;
	Mesh monkeyMesh;

	std::vector<RenderObject> renderables;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;

	DeletionQueue mainDeletionQueue;

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();

private:

	void InitVulkan();

	void InitSwapchain();

	void InitCommands();

	void InitDefaultRenderpass();

	void InitFramebuffers();

	void InitSyncStructures();

	void InitPipelines();

	void InitScene();

	bool LoadShaderModule(const char* filePath, VkShaderModule* outShaderModule);

	void LoadMeshes();

	void UploadMesh(Mesh& mesh);

	Material* CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	Material* GetMaterial(const std::string& name);

	Mesh* GetMesh(const std::string& name);

	void DrawObjects(VkCommandBuffer cmd, RenderObject* first, int count);

};





// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>

#include <vector>
#include <deque>
#include <functional>

class PipelineBuilder {
public:

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
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

class VulkanEngine {
public:

	VulkanEngine();

	bool isInitialized;
	int frameNumber;
	int selectedShader;

	VkExtent2D windowExtent;
	struct SDL_Window* window{ nullptr };
	

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkSurfaceKHR surface;

	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;

	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;

	VkPipelineLayout trianglePipelineLayout;
	VkPipeline trianglePipeline;
	VkPipeline redTrianglePipeline;

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

	bool LoadShaderModule(const char* filePath, VkShaderModule* outShaderModule);
};





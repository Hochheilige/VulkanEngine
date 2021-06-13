#pragma once

#include <Window.hpp>
#include <Swapchain.hpp>
#include <Image.hpp>
#include <Buffer.hpp>
#include <DescriptorSet.hpp>

class Engine {
public:
	Engine();

	~Engine();

	void run();
private:
	Window* window = nullptr;
	VulkanBase vulkanBase;
};
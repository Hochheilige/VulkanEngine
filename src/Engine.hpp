#pragma once

#include <Window.hpp>
#include <VulkanBase.hpp>

class Engine {
public:
	Engine();

	~Engine();

	void run();
private:
	Window* window = nullptr;
	VulkanBase vulkanBase;
};
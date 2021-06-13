#pragma once

#include <Window.hpp>
#include <VulkanBase.hpp>

class Engine {
public:
	Engine();

	~Engine();

	void run();
private:
	std::unique_ptr<Window> window = nullptr;
	std::unique_ptr<VulkanBase> vulkanBase;
};
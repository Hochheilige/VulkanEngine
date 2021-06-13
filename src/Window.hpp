#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <cstdint>

class Window {
public:
	Window() = delete;
	Window(const Window& other) = delete;
	void operator= (const Window& window) = delete;

	static Window* CreateWindow(const char* name, const uint32_t width, const uint32_t height);

	bool PollEvents(SDL_Event* event);

	SDL_Window* GetWindow();

	bool isShouldClose = false;
private:
	static Window* instance;
	SDL_Window* window = nullptr;

	Window(const char* name, const uint32_t width, const uint32_t height);
	~Window();
};


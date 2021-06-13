#include "Window.hpp"

Window* Window::instance{ nullptr };

Window* Window::CreateWindow(const char* name, const uint32_t width, const uint32_t height) {
	if (instance == nullptr) {
		instance = new Window(name, width, height);
	}
	return instance;
}

bool Window::PollEvents(SDL_Event* event) {
	return SDL_PollEvent(event);
}

SDL_Window* Window::GetWindow() {
	return window;
}

Window::Window(const char* name, const uint32_t width, const uint32_t height) {
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(
		name,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_VULKAN
	);
}

Window::~Window() {
	SDL_DestroyWindow(window);
}

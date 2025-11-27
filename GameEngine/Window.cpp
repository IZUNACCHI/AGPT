#include "Window.h"
#include "EngineException.h"
#include <SDL3/SDL.h>

struct Window::Impl {
	SDL_Window* sdlWindow = nullptr;
	int width = 0;
	int height = 0;

	Impl(const std::string& title, int w, int h) {
		sdlWindow = SDL_CreateWindow(title.c_str(), w, h, SDL_WINDOW_RESIZABLE);
		width = w;
		height = h;
		if (!sdlWindow) throw EngineException(SDL_GetError());
		//TODO: Width Height invalid exceptions
	}

	~Impl() {
		if (sdlWindow) SDL_DestroyWindow(sdlWindow);
	}
};

Window::Window(const std::string& title, int w, int h)
	: impl(std::make_unique<Impl>(title, w, h)) {
}

Window::~Window() = default;

int Window::Width() const { return impl->width; }

int Window::Height() const { return impl->height; }

void* Window::GetNative() const { return impl->sdlWindow; }
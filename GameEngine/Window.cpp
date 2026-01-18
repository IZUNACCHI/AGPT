#include "Window.h"
#include "EngineException.hpp"
#include "Logger.h"
#include <SDL3/SDL.h>

namespace Engine {

	struct Window::Impl {
		SDL_Window* window = nullptr;
		WindowConfig config;

		~Impl() {
			if (window) {
				SDL_DestroyWindow(window);
			}
		}
	};

	Window::Window(const WindowConfig& config)
		: impl(std::make_unique<Impl>()) {

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
			impl.reset();  // Reset unique_ptr before throwing
			THROW_ENGINE_EXCEPTION("Failed to initialize SDL: ") << SDL_GetError();
		}
		impl->config = config;

		uint32_t flags = 0;
		if (config.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
		if (config.borderless) flags |= SDL_WINDOW_BORDERLESS;
		if (config.resizable) flags |= SDL_WINDOW_RESIZABLE;

		impl->window = SDL_CreateWindow(
			config.title.c_str(),
			config.windowSize.x,
			config.windowSize.y,
			flags
		);

		if (!impl->window) {
			impl.reset();  // Reset unique_ptr before throwing
			THROW_ENGINE_EXCEPTION("Failed to create window: ") << SDL_GetError();
		}
	}

	Window::~Window() = default;  

	Window::Window(Window&& other) noexcept
		: impl(std::move(other.impl)) {
	}

	Window& Window::operator=(Window&& other) noexcept {
		if (this != &other) {
			impl = std::move(other.impl);
		}
		return *this;
	}

	Vector2i Window::GetSize() const {
		Vector2i windowSize(0, 0);
		if (impl && impl->window) {
			SDL_GetWindowSize(impl->window, &windowSize.x, &windowSize.y);
		}
		return windowSize;
	}

	std::string Window::GetTitle() const {
		return impl ? impl->config.title : "";
	}

	bool Window::IsFullscreen() const {
		return impl && impl->config.fullscreen;
	}

	bool Window::IsBorderless() const {
		return impl && impl->config.borderless;
	}

	bool Window::IsResizable() const {
		return impl && impl->config.resizable;
	}

	bool Window::IsVisible() const {
		if (!impl || !impl->window) return false;

		uint32_t flags = SDL_GetWindowFlags(impl->window);
		return (flags & SDL_WINDOW_HIDDEN) == 0;
	}

	void Window::SetTitle(const std::string& title) {
		if (!impl || !impl->window) return;

		impl->config.title = title;
		SDL_SetWindowTitle(impl->window, title.c_str());
	}

	void Window::SetSize(const Vector2i& windowSize) {
		if (!impl || !impl->window) return;

		impl->config.windowSize = windowSize;
		SDL_SetWindowSize(impl->window, windowSize.x, windowSize.y);
	}

	void Window::SetFullscreen(bool fullscreen) {
		if (!impl || !impl->window) return;

		impl->config.fullscreen = fullscreen;
		SDL_SetWindowFullscreen(impl->window, fullscreen);
	}

	void Window::SetVisible(bool visible) {
		if (!impl || !impl->window) return;

		if (visible) {
			SDL_ShowWindow(impl->window);
		}
		else {
			SDL_HideWindow(impl->window);
		}
	}

	void Window::Minimize() {
		if (impl && impl->window) {
			SDL_MinimizeWindow(impl->window);
		}
	}

	void Window::Maximize() {
		if (impl && impl->window) {
			SDL_MaximizeWindow(impl->window);
		}
	}

	void Window::Restore() {
		if (impl && impl->window) {
			SDL_RestoreWindow(impl->window);
		}
	}

	void Window::Show() {
		SetVisible(true);
	}

	void Window::Hide() {
		SetVisible(false);
	}

	void Window::Focus() {
		if (impl && impl->window) {
			SDL_RaiseWindow(impl->window);
		}
	}

	void* Window::GetNative() const {
		return impl ? static_cast<void*>(impl->window) : nullptr;
	}

	void Window::SetMouseGrab(bool grab) {
		if (impl && impl->window) {
			SDL_SetWindowMouseGrab(impl->window, grab ? true : false);
		}
	}

	void Window::SetMouseRelativeMode(bool relative) {
		if (impl && impl->window) {
			SDL_SetWindowRelativeMouseMode(impl->window, relative ? true : false);
		}
	}

} // namespace Engine
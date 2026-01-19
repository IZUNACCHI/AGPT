#include "Renderer.h"


	Renderer::Renderer(Window& window)
		: m_renderer(nullptr) {

		LOG_INFO("Initializing renderer");

		// Get the SDL window from the Window class
		SDL_Window* sdlWindow = static_cast<SDL_Window*>(window.GetNative());
		if (!sdlWindow) {
			THROW_ENGINE_EXCEPTION("Cannot create renderer - window is not valid");
		}

		// Create SDL renderer
		m_renderer = SDL_CreateRenderer(sdlWindow, nullptr);

		if (!m_renderer) {
			THROW_ENGINE_EXCEPTION("Failed to create renderer: ") << SDL_GetError();
		}

		LOG_INFO("Renderer initialized successfully");
	}

	Renderer::Renderer(Renderer&& other) noexcept
		: m_renderer(other.m_renderer) {
		other.m_renderer = nullptr;
	}

	Renderer& Renderer::operator=(Renderer&& other) noexcept {
		if (this != &other) {
			// Clean up current renderer
			if (m_renderer) {
				SDL_DestroyRenderer(m_renderer);
			}

			m_renderer = other.m_renderer;
			other.m_renderer = nullptr;
		}
		return *this;
	}

	Renderer::~Renderer() {
		if (m_renderer) {
			SDL_DestroyRenderer(m_renderer);
			LOG_INFO("Renderer destroyed");
		}
	}

	

	void Renderer::Clear() {
		if (!m_renderer) {
			LOG_WARN("Cannot clear - renderer is not valid");
			return;
		}

		// Clear the screen
		SDL_RenderClear(m_renderer);
	}

	void Renderer::Present() {
		if (!m_renderer) {
			LOG_WARN("Cannot present - renderer is not valid");
			return;
		}

		SDL_RenderPresent(m_renderer);
	}

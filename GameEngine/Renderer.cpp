#include "Renderer.h"
#include "Texture.h"

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
		: m_renderer(other.m_renderer){
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

	bool Renderer::DrawTexture(const Texture& texture,
		const Vector2f& sourcePosition,
		const Vector2f& sourceSize,
		const Vector2f& destinationPosition,
		const Vector2f& destinationSize) {
		if (!m_renderer) {
			LOG_WARN("Cannot draw texture - renderer is not valid");
			return false;
		}
		if (!texture.IsValid()) {
			LOG_WARN("Cannot draw texture - texture is not valid");
			return false;
		}

		const SDL_FRect src{ sourcePosition.x, sourcePosition.y, sourceSize.x, sourceSize.y };
		Vector2f screenPosition = destinationPosition;
		int outputWidth = 0;
		int outputHeight = 0;
		if (SDL_GetRenderOutputSize(m_renderer, &outputWidth, &outputHeight)) {
			screenPosition.x = destinationPosition.x + (static_cast<float>(outputWidth) * 0.5f);
			screenPosition.y = (static_cast<float>(outputHeight) * 0.5f) - destinationPosition.y;
		}
		const SDL_FRect dst{ screenPosition.x, screenPosition.y, destinationSize.x, destinationSize.y };

		if (!SDL_RenderTexture(m_renderer, texture.GetNative(), &src, &dst)) {
			LOG_WARN("Renderer draw texture failed: " + std::string(SDL_GetError()));
			return false;
		}

		return true;
	}
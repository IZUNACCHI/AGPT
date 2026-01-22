#include "Surface.h"


    Surface::Surface(const std::string& filePath) {
        m_surface = SDL_LoadBMP(filePath.c_str());
        if (!m_surface) {
            THROW_ENGINE_EXCEPTION("Failed to load BMP: ") << SDL_GetError() << " (File: " << filePath << ")";
        }
    }

    Surface::Surface(Surface&& other) noexcept
        : m_surface(other.m_surface) {
        other.m_surface = nullptr;
    }

    Surface& Surface::operator=(Surface&& other) noexcept {
        if (this != &other) {
            if (m_surface) {
                SDL_DestroySurface(m_surface);
            }

            m_surface = other.m_surface;
            other.m_surface = nullptr;
        }
        return *this;
    }

    Surface::~Surface() {
        if (m_surface) {
            SDL_DestroySurface(m_surface);
        }
    }
    
	void Surface::SetColorKey(const Vector3i& color) {
		if (!m_surface) {
			THROW_ENGINE_EXCEPTION("Cannot set color key on invalid surface");
		}

		if (!SDL_SetSurfaceColorKey(m_surface, true,
			SDL_MapSurfaceRGB(m_surface, color.x, color.y, color.z))) {
			THROW_ENGINE_EXCEPTION("Failed to set color key: ") << SDL_GetError();
		}
	}

    void Surface::ClearColorKey() {
        if (!m_surface) {
            return;
        }

        SDL_SetSurfaceColorKey(m_surface, false, 0);
        LOG_DEBUG("Color key cleared");
    }

    Vector2i Surface::GetSize() const {
        if (!m_surface) {
            return Vector2i(0, 0);
        }
        return Vector2i(m_surface->w, m_surface->h);
    }

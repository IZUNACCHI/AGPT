#include "Surface.h"
#include <SDL3/SDL.h>



    Surface::Surface(const std::string& filePath) {
        m_surface = (void*)SDL_LoadBMP(filePath.c_str());
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
                SDL_DestroySurface(static_cast<SDL_Surface*>(m_surface));
            }

            m_surface = other.m_surface;
            other.m_surface = nullptr;
        }
        return *this;
    }

    Surface::~Surface() {
        if (m_surface) {
            SDL_DestroySurface(static_cast<SDL_Surface*>(m_surface));
        }
    }
    
	void Surface::SetColorKey(const Vector3i& color) {
		if (!m_surface) {
			THROW_ENGINE_EXCEPTION("Cannot set color key on invalid surface");
		}

		auto colorKey = SDL_MapSurfaceRGB(static_cast<SDL_Surface*>(m_surface), color.x, color.y, color.z);
		if (!SDL_SetSurfaceColorKey(static_cast<SDL_Surface*>(m_surface), true, colorKey)) {
			THROW_ENGINE_EXCEPTION("Failed to set color key: ") << SDL_GetError();
		}
	}

    void Surface::ClearColorKey() {
        if (!m_surface) {
            return;
        }

        SDL_SetSurfaceColorKey(static_cast<SDL_Surface*>(m_surface), false, 0);
        LOG_DEBUG("Color key cleared");
    }

    Vector2i Surface::GetSize() const {
        if (!m_surface) {
            return Vector2i(0, 0);
        }
        SDL_Surface* s = static_cast<SDL_Surface*>(m_surface);
		return Vector2i(s->w, s->h);
    }

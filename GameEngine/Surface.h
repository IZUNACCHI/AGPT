#pragma once

#include "Types.hpp"
#include "EngineException.hpp"
#include "Logger.h"
#include <SDL3/SDL.h>

namespace Engine {

	class Surface {
	public:
		// Construct from BMP file
		Surface(const std::string& filePath);

		// No copying
		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;

		// Move semantics
		Surface(Surface&& other) noexcept;
		Surface& operator=(Surface&& other) noexcept;

		~Surface();

		void SetColorKey(const Vector3i& color);
		void ClearColorKey();

		// Getters
		SDL_Surface* GetNative() const { return m_surface; }
		Vector2i GetSize() const;

		// Check if valid
		bool IsValid() const { return m_surface != nullptr; }

	private:
		SDL_Surface* m_surface = nullptr;
	};

} // namespace Engine
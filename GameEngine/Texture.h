#pragma once

#include "Types.hpp"
#include "EngineException.hpp"
#include "Logger.h"
#include "Surface.h"
#include <SDL3/SDL.h>
#include <memory>

namespace Engine {

	class Renderer;

	class Texture {
	public:
		// Pimpl forward declaration
		struct Impl;

		// Constructor without color key (loads as-is)
		Texture(Renderer& renderer, const std::string& filePath);

		// Constructor with Vector3i color key
		Texture(Renderer& renderer, const std::string& filePath, const Vector3i* colorKey);

		// Destructor
		~Texture();

		// No copying
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		// Move semantics
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		// Getters
		Vector2i GetSize() const;
		SDL_Texture* GetNative() const;
		bool IsValid() const;

	private:
		Texture(Renderer& renderer, const std::string& filePath, bool useColorKey, const Vector3i* colorKey);

		std::unique_ptr<Impl> impl;
	};

} // namespace Engine
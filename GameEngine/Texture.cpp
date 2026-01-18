#include "Texture.h"
#include "Renderer.h"

namespace Engine {

	struct Texture::Impl {
		SDL_Texture* texture = nullptr;
		Vector2i windowSize;

		// Constructor for Vector3i color key
		Impl(Renderer& renderer, const std::string& filePath, bool useColorKey, const Vector3i* colorKey) {
			// Create surface from file
			Surface surface(filePath);

			// Apply color key if requested
			if (useColorKey && colorKey) {
				// Dereference the pointer to get Vector3i reference
				surface.SetColorKey(*colorKey);
			}

			// Create texture from surface
			texture = SDL_CreateTextureFromSurface(
				renderer.GetNative(),
				surface.GetNative()
			);

			if (!texture) {
				THROW_ENGINE_EXCEPTION("Failed to create texture: ") << SDL_GetError();
			}

			// Get texture size
			windowSize = surface.GetSize();
		}

		~Impl() {
			if (texture) {
				SDL_DestroyTexture(texture);
				LOG_DEBUG("Texture destroyed");
			}
		}

		// No copying
		Impl(const Impl&) = delete;
		Impl& operator=(const Impl&) = delete;

		// Move semantics
		Impl(Impl&& other) noexcept : texture(other.texture), windowSize(other.windowSize) {
			other.texture = nullptr;
		}

		Impl& operator=(Impl&& other) noexcept {
			if (this != &other) {
				// Clean up current texture
				if (texture) {
					SDL_DestroyTexture(texture);
				}

				texture = other.texture;
				windowSize = other.windowSize;
				other.texture = nullptr;
			}
			return *this;
		}
	};


	// Private helper constructor for Vector3i
	Texture::Texture(Renderer& renderer, const std::string& filePath, bool useColorKey, const Vector3i* colorKey)
		: impl(std::make_unique<Impl>(renderer, filePath, useColorKey, colorKey)) {
	}

	// Constructor without color key
	Texture::Texture(Renderer& renderer, const std::string& filePath)
		: Texture(renderer, filePath, false, nullptr) {
	}

	// Constructor with Vector3i color key
	Texture::Texture(Renderer& renderer, const std::string& filePath, const Vector3i* colorKey)
		: Texture(renderer, filePath, true, colorKey) {
	}

	Texture::~Texture() = default;  // unique_ptr handles destruction

	Texture::Texture(Texture&& other) noexcept
		: impl(std::move(other.impl)) {
	}

	Texture& Texture::operator=(Texture&& other) noexcept {
		if (this != &other) {
			impl = std::move(other.impl);
		}
		return *this;
	}

	Vector2i Texture::GetSize() const {
		return impl ? impl->windowSize : Vector2i(0, 0);
	}

	SDL_Texture* Texture::GetNative() const {
		return impl ? impl->texture : nullptr;
	}

	bool Texture::IsValid() const {
		return impl && impl->texture != nullptr;
	}

} // namespace Engine
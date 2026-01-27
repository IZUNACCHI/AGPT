#include "Texture.h"
#include "Renderer.h"
#include <SDL3/SDL.h>

struct Texture::Impl {
	SDL_Texture* texture = nullptr;
	Vector2i windowSize{};
	TextureScaleMode scaleMode = TextureScaleMode::Linear;

	Impl(Renderer& renderer, const std::string& filePath, bool useColorKey, const Vector3i& colorKey) {
		Surface surface(filePath);

		if (useColorKey) {
			surface.SetColorKey(colorKey);
		}

		texture = SDL_CreateTextureFromSurface(
			static_cast<SDL_Renderer*>(renderer.GetNative()),
			static_cast<SDL_Surface*>(surface.GetNative())
		);

		if (!texture) {
			THROW_ENGINE_EXCEPTION("Failed to create texture: ") << SDL_GetError();
		}

		windowSize = surface.GetSize();

		// Default filtering linear
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
		scaleMode = TextureScaleMode::Linear;
	}

	~Impl() {
		if (texture) {
			SDL_DestroyTexture(texture);
		}
	}

	Impl(const Impl&) = delete;
	Impl& operator=(const Impl&) = delete;

	Impl(Impl&& other) noexcept
		: texture(other.texture),
		  windowSize(other.windowSize),
		  scaleMode(other.scaleMode) {
		other.texture = nullptr;
	}

	Impl& operator=(Impl&& other) noexcept {
		if (this != &other) {
			if (texture) {
				SDL_DestroyTexture(texture);
			}
			texture = other.texture;
			windowSize = other.windowSize;
			scaleMode = other.scaleMode;
			other.texture = nullptr;
		}
		return *this;
	}
};

static SDL_ScaleMode ToSDLScaleMode(TextureScaleMode mode) {
	switch (mode) {
	case TextureScaleMode::Nearest: return SDL_SCALEMODE_NEAREST;
	case TextureScaleMode::Linear:  return SDL_SCALEMODE_LINEAR;
	default:                        return SDL_SCALEMODE_LINEAR;
	}
}

Texture::Texture(Renderer& renderer, const std::string& filePath)
	: impl(std::make_unique<Impl>(renderer, filePath, false, Vector3i(0, 0, 0))) {}

Texture::Texture(Renderer& renderer, const std::string& filePath, bool useColorKey, const Vector3i& colorKey)
	: impl(std::make_unique<Impl>(renderer, filePath, useColorKey, colorKey)) {}

Texture::~Texture() = default;

Texture::Texture(Texture&& other) noexcept = default;
Texture& Texture::operator=(Texture&& other) noexcept = default;

Vector2i Texture::GetSize() const {
	return impl ? impl->windowSize : Vector2i(0, 0);
}

void* Texture::GetNative() const {
	return impl ? (void*)impl->texture : nullptr;
}

bool Texture::IsValid() const {
	return impl && impl->texture != nullptr;
}

void Texture::SetScaleMode(TextureScaleMode mode) {
	if (!impl || !impl->texture) return;
	SDL_SetTextureScaleMode(impl->texture, ToSDLScaleMode(mode));
	impl->scaleMode = mode;
}

TextureScaleMode Texture::GetScaleMode() const {
	return impl ? impl->scaleMode : TextureScaleMode::Linear;
}

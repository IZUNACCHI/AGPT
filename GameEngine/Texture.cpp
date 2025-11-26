#include "Texture.h"
#include "Renderer.h"
#include "EngineException.h"

#include <SDL3/SDL.h>

struct Texture::Impl {
	SDL_Texture* tex = nullptr;
	int width = 0;
	int height = 0;

	Impl(Renderer& renderer, const std::string& path) {
		SDL_Surface* surf = SDL_LoadBMP(path.c_str());
		if (!surf) throw EngineException(SDL_GetError());

		tex = SDL_CreateTextureFromSurface(
			(SDL_Renderer*)renderer.GetNative(), surf
		);
		if (!tex) {
			SDL_DestroySurface(surf);
			throw EngineException(SDL_GetError());
		}

		width = surf->w;
		height = surf->h;

		SDL_DestroySurface(surf);
	}

	~Impl() {
		if (tex) SDL_DestroyTexture(tex);
	}
};

Texture::Texture(Renderer& renderer, const std::string& path)
	: impl(std::make_unique<Impl>(renderer, path)) {
}

Texture::~Texture() = default;

int Texture::Width() const { return impl->width; }
int Texture::Height() const { return impl->height; }
void* Texture::GetNative() const { return impl->tex; }

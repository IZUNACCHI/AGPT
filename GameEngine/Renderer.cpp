#include "Renderer.h"
#include "EngineException.h"
#include <SDL3/SDL.h>
#include "Texture.h"

struct Renderer::Impl {
	SDL_Renderer* sdlRenderer = nullptr;

	Impl(Window& w) {
		sdlRenderer = SDL_CreateRenderer((SDL_Window*)w.GetNative(), nullptr);
		if (!sdlRenderer) throw EngineException(SDL_GetError());
	}

	~Impl() {
		if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
	}
};

Renderer::Renderer(Window& w)
	: impl(std::make_unique<Impl>(w)) {
}

Renderer::~Renderer() = default;

void Renderer::Clear() {
	SDL_RenderClear((SDL_Renderer*)impl->sdlRenderer);
}

void Renderer::Present() {
	SDL_RenderPresent((SDL_Renderer*)impl->sdlRenderer);
}

void* Renderer::GetNative() const {
	return impl->sdlRenderer;
}

void Renderer::DrawTexture(Texture& tex, const Rect& src, const Rect& dst, Flip flip) {
	SDL_FRect s = { (float)src.x, (float)src.y, (float)src.w, (float)src.h };
	SDL_FRect d = { (float)dst.x, (float)dst.y, (float)dst.w, (float)dst.h };

	SDL_FlipMode sdlFlip = SDL_FLIP_NONE;
	switch (flip) {
	case Flip::Horizontal: sdlFlip = SDL_FLIP_HORIZONTAL; break;
	case Flip::Vertical: sdlFlip = SDL_FLIP_VERTICAL; break;
	case Flip::Both: sdlFlip = (SDL_FlipMode)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL); break;
	default: break;
	}

	SDL_RenderTextureRotated((SDL_Renderer*)impl->sdlRenderer, (SDL_Texture*)tex.GetNative(),
		&s, &d, 0.0, nullptr, sdlFlip);
}
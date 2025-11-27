// Modify Renderer.cpp accordingly

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

void Renderer::DrawTexture(Texture& tex, const FRect* src, const FRect& dst, double angle /*= 0.0*/, Flip flip /*= Flip::None*/)
{
	SDL_FRect dstRect = { dst.x, dst.y, dst.w, dst.h };

	SDL_FlipMode sdlFlip = SDL_FLIP_NONE;
	if (flip == Flip::Horizontal)      sdlFlip = SDL_FLIP_HORIZONTAL;
	else if (flip == Flip::Vertical)   sdlFlip = SDL_FLIP_VERTICAL;
	else if (flip == Flip::Both)       sdlFlip = (SDL_FlipMode)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);

	if (src)
	{
		SDL_FRect srcRect = { src->x, src->y, src->w, src->h };
		SDL_RenderTextureRotated(impl->sdlRenderer, (SDL_Texture*)tex.GetNative(), &srcRect, &dstRect, angle, nullptr, sdlFlip);
	}
	else
	{
		SDL_RenderTextureRotated(impl->sdlRenderer, (SDL_Texture*)tex.GetNative(), nullptr, &dstRect, angle, nullptr, sdlFlip);
	}
}
#include "Renderer.h"
#include "Texture.h"
#include <string>
#include <cmath>

Renderer::Renderer(Window& window)
	: m_renderer(nullptr) {

	LOG_INFO("Initializing renderer");

	SDL_Window* sdlWindow = static_cast<SDL_Window*>(window.GetNative());
	if (!sdlWindow) {
		THROW_ENGINE_EXCEPTION("Cannot create renderer - window is not valid");
	}

	m_renderer = SDL_CreateRenderer(sdlWindow, nullptr);
	if (!m_renderer) {
		THROW_ENGINE_EXCEPTION("Failed to create renderer: ") << SDL_GetError();
	}

	LOG_INFO("Renderer initialized successfully");
}

Renderer::Renderer(Renderer&& other) noexcept
	: m_renderer(other.m_renderer) {
	other.m_renderer = nullptr;
}

Renderer& Renderer::operator=(Renderer&& other) noexcept {
	if (this != &other) {
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
	if (!m_renderer) return;
	SDL_RenderClear(m_renderer);
}

void Renderer::Present() {
	if (!m_renderer) return;
	SDL_RenderPresent(m_renderer);
}

bool Renderer::GetOutputSize(int& outW, int& outH) const {
	outW = 0; outH = 0;
	if (!m_renderer) return false;
	return SDL_GetRenderOutputSize(m_renderer, &outW, &outH);
}

Vector2f Renderer::WorldToScreenPoint(const Vector2f& world) const {
	int w = 0, h = 0;
	if (!GetOutputSize(w, h)) {
		return world; // fallback
	}

	// WORLD: origin center, +Y up
	// SCREEN: origin top-left, +Y down
	return Vector2f(
		world.x + (float)w * 0.5f,
		(float)h * 0.5f - world.y
	);
}

SDL_FRect Renderer::WorldToScreenRect(const Vector2f& worldTopLeft, const Vector2f& size) const {
	const Vector2f screenTL = WorldToScreenPoint(worldTopLeft);
	SDL_FRect r{};
	r.x = screenTL.x;
	r.y = screenTL.y;
	r.w = size.x;
	r.h = size.y;
	return r;
}

bool Renderer::DrawTexture(
	const Texture& texture,
	const Vector2f& sourcePosition,
	const Vector2f& sourceSize,
	const Vector2f& destinationPosition,
	const Vector2f& destinationSize) {

	if (!m_renderer) return false;
	if (!texture.IsValid()) return false;

	const SDL_FRect src{ sourcePosition.x, sourcePosition.y, sourceSize.x, sourceSize.y };
	const SDL_FRect dst = WorldToScreenRect(destinationPosition, destinationSize);

	if (!SDL_RenderTexture(m_renderer, texture.GetNative(), &src, &dst)) {
		LOG_WARN("Renderer draw texture failed: " + std::string(SDL_GetError()));
		return false;
	}
	return true;
}

bool Renderer::DrawTextureRotated(
	const Texture& texture,
	const Vector2f& sourcePosition,
	const Vector2f& sourceSize,
	const Vector2f& destinationPosition,
	const Vector2f& destinationSize,
	float angleDegrees,
	const Vector2f& pivot,
	SDL_FlipMode flip) {

	if (!m_renderer) return false;
	if (!texture.IsValid()) return false;

	const SDL_FRect src{ sourcePosition.x, sourcePosition.y, sourceSize.x, sourceSize.y };
	const SDL_FRect dst = WorldToScreenRect(destinationPosition, destinationSize);

	SDL_FPoint center{};
	if (pivot.x < 0.0f || pivot.y < 0.0f) {
		center.x = dst.w * 0.5f;
		center.y = dst.h * 0.5f;
	}
	else {
		center.x = pivot.x;
		center.y = pivot.y;
	}

	// Preserve WORLD CCW meaning with Y-down screen
	const float sdlAngle = -angleDegrees;

	if (!SDL_RenderTextureRotated(m_renderer, texture.GetNative(), &src, &dst, sdlAngle, &center, flip)) {
		LOG_WARN("Renderer draw rotated texture failed: " + std::string(SDL_GetError()));
		return false;
	}
	return true;
}

bool Renderer::DrawRectOutline(const Vector2f& worldTopLeft, const Vector2f& size, const Vector3i& color) {
	if (!m_renderer) return false;

	if (!SDL_SetRenderDrawColor(m_renderer, color.x, color.y, color.z, 255)) {
		LOG_WARN("Renderer draw rect failed to set color: " + std::string(SDL_GetError()));
		return false;
	}

	const SDL_FRect rect = WorldToScreenRect(worldTopLeft, size);
	if (!SDL_RenderRect(m_renderer, &rect)) {
		LOG_WARN("Renderer draw rect failed: " + std::string(SDL_GetError()));
		return false;
	}

	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	return true;
}

bool Renderer::DrawCircleOutline(const Vector2f& worldCenter, float radius, const Vector3i& color, int segments) {
	if (!m_renderer) return false;
	if (radius <= 0.0f || segments < 3) return false;

	if (!SDL_SetRenderDrawColor(m_renderer, color.x, color.y, color.z, 255)) {
		LOG_WARN("Renderer draw circle failed to set color: " + std::string(SDL_GetError()));
		return false;
	}

	const Vector2f c = WorldToScreenPoint(worldCenter);

	const float step = Math::Constants<float>::TwoPi / (float)segments;
	float a = 0.0f;

	float prevX = c.x + radius;
	float prevY = c.y;

	for (int i = 1; i <= segments; ++i) {
		a += step;

		// screen y is down, so use -sin to match world +Y up
		const float x = c.x + std::cos(a) * radius;
		const float y = c.y - std::sin(a) * radius;

		SDL_RenderLine(m_renderer, prevX, prevY, x, y);
		prevX = x;
		prevY = y;
	}

	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	return true;
}

#pragma once

#include "Types.hpp"
#include "EngineException.hpp"
#include "Logger.h"
#include "Window.h"
#include <SDL3/SDL.h>

class Texture;

class Renderer {
public:
	explicit Renderer(Window& window);

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Renderer(Renderer&& other) noexcept;
	Renderer& operator=(Renderer&& other) noexcept;

	~Renderer();

	void Clear();
	void Present();

	// WORLD coords (Box2D style):
	// - origin is screen center
	// - +Y is up
	// destinationPosition is WORLD top-left of destination rect.
	bool DrawTexture(
		const Texture& texture,
		const Vector2f& sourcePosition,
		const Vector2f& sourceSize,
		const Vector2f& destinationPosition,
		const Vector2f& destinationSize
	);

	// Rotated draw in WORLD coords:
	// - angleDegrees is CCW in WORLD.
	// - pivot is in DEST rect local pixels (default: center if pivot.x/y < 0)
	bool DrawTextureRotated(
		const Texture& texture,
		const Vector2f& sourcePosition,
		const Vector2f& sourceSize,
		const Vector2f& destinationPosition,
		const Vector2f& destinationSize,
		float angleDegrees,
		const Vector2f& pivot = Vector2f(-1.0f, -1.0f),
		SDL_FlipMode flip = SDL_FLIP_NONE
	);

	bool DrawRectOutline(const Vector2f& position, const Vector2f& size, const Vector3i& color);
	bool DrawCircleOutline(const Vector2f& center, float radius, const Vector3i& color, int segments);

	SDL_Renderer* GetNative() const { return m_renderer; }
	bool IsValid() const { return m_renderer != nullptr; }

private:
	bool GetOutputSize(int& outW, int& outH) const;

	Vector2f WorldToScreenPoint(const Vector2f& world) const;
	SDL_FRect WorldToScreenRect(const Vector2f& worldTopLeft, const Vector2f& size) const;

private:
	SDL_Renderer* m_renderer = nullptr;
};

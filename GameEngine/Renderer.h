#pragma once

#include "Types.hpp"
#include "EngineException.hpp"
#include "Logger.h"
#include "Window.h"
#include <SDL3/SDL.h>

class Renderer {
public:
	// Constructor takes a window (must already be initialized)
	explicit Renderer(Window& window);

	// No copying
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	// Move semantics
	Renderer(Renderer&& other) noexcept;
	Renderer& operator=(Renderer&& other) noexcept;

	~Renderer();

	// Renderer operations
	void Clear();
	void Present();

	// Get native SDL renderer
	SDL_Renderer* GetNative() const { return m_renderer; }

	// Check if valid
	bool IsValid() const { return m_renderer != nullptr; }

private:
	SDL_Renderer* m_renderer = nullptr;
};
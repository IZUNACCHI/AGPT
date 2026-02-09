#pragma once

#include "Types.hpp"
#include "EngineException.hpp"
#include "Logger.h"
#include "Window.h"
#include <cstdint>

class Texture;


// Flip options for rotated draws
enum class FlipMode {
	None,
	Horizontal,
	Vertical,
	Both
};

/// How the virtual resolution is mapped to the real window.
/// - Letterbox: preserve aspect, show bars when needed ("contain")
/// - Stretch: fill the whole window, distort aspect ("stretch")
/// - Crop: preserve aspect, fill the whole window, crop edges ("cover")
enum class ViewportScaleMode {
	Letterbox,
	Stretch,
	Crop
};

// Renderer: handles drawing to a window with optional virtual resolution and letterboxing
class Renderer {
public:
	explicit Renderer(Window& window);

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Renderer(Renderer&& other) noexcept;
	Renderer& operator=(Renderer&& other) noexcept;

	~Renderer();

	/// Clear the frame
	/// - Clears the full window to the letterbox color
	/// - Then clears the active viewport (game area) to the clear color
	void Clear();


	void BeginFrame();
	void Present();
	void EndFrame();

	// Virtual Resolution / Letterboxed Viewport
	// 
	// If a virtual resolution is set (> 0), all WORLD coordinates and sizes
	// are interpreted in that fixed coordinate space. When the real window
	// size changes,then scale the virtual space to fit while preserving aspect
	// ratio, adding letterbox bars where needed.
	//
	// If virtual resolution is (0,0), the renderer behaves as before:
	// WORLD units == window pixels.

	void SetVirtualResolution(int width, int height);
	void SetVirtualResolution(const Vector2i& size);
	Vector2i GetVirtualResolution() const;

	// Letterboxing is ON by default when virtual resolution is set.
	void SetLetterbox(bool enabled);
	bool IsLetterboxEnabled() const { return m_scaleMode == ViewportScaleMode::Letterbox; }

	void SetViewportScaleMode(ViewportScaleMode mode);
	ViewportScaleMode GetViewportScaleMode() const { return m_scaleMode; }

	// If enabled, and the scale is >= 1,  floor the scale to an integer.
	// Useful for pixel art. If the window is smaller than the virtual size,
	// we fall back to fractional scaling to avoid scale=0.
	void SetIntegerScaling(bool enabled);
	bool IsIntegerScaling() const { return m_integerScale; }

	// Colors
	// - ClearColor: the game area background color (inside the viewport)
	// - LetterboxColor: the bars/background outside the viewport
	void SetClearColor(const Vector4i& rgba);
	void SetLetterboxColor(const Vector4i& rgba);
	Vector4i GetClearColor() const { return m_clearColor; }
	Vector4i GetLetterboxColor() const { return m_letterboxColor; }

	// The destination rectangle (in real window pixels) where the game is
	// rendered when using a virtual resolution.
	Rectf GetViewportRect() const;

	// WORLD coords:
	// - (0,0) is screen center
	// - +Y is up
	// destinationPosition is WORLD TOP-LEFT
	bool DrawTexture(
		const Texture& texture,
		const Vector2f& sourcePosition,
		const Vector2f& sourceSize,
		const Vector2f& destinationPosition,
		const Vector2f& destinationSize
	);

	// Same as DrawTexture, but temporarily applies a tint (RGBA 0-255) to the texture.
	// This restores the previous modulation after the draw, so shared textures remain safe.
	bool DrawTextureTinted(
		const Texture& texture,
		const Vector2f& sourcePosition,
		const Vector2f& sourceSize,
		const Vector2f& destinationPosition,
		const Vector2f& destinationSize,
		const Vector4i& tint
	);

	// angleDegrees: CCW positive in WORLD
	// pivot: local pixels inside destination rect (default center if pivot < 0)
	bool DrawTextureRotated(
		const Texture& texture,
		const Vector2f& sourcePosition,
		const Vector2f& sourceSize,
		const Vector2f& destinationPosition,
		const Vector2f& destinationSize,
		float angleDegrees,
		const Vector2f& pivot = Vector2f(-1.0f, -1.0f),
		FlipMode flip = FlipMode::None
	);

	// WORLD top-left rect
	bool DrawRectOutline(const Vector2f& worldTopLeft, const Vector2f& size, const Vector3i& color);

	// WORLD center, rotated CCW by angleDegrees
	bool DrawRectOutlineRotated(const Vector2f& worldCenter, const Vector2f& size, float angleDegrees, const Vector3i& color);

	// WORLD top-left filled rect (supports alpha)
	bool DrawFilledRect(const Vector2f& worldTopLeft, const Vector2f& size, const Vector4i& color);

	// WORLD center
	bool DrawCircleOutline(const Vector2f& worldCenter, float radius, const Vector3i& color, int segments);

	// Native handle access (native* as void*)
	void* GetNative() const;

	bool IsValid() const { return m_renderer != nullptr; }

private:
	bool GetOutputSize(int& outW, int& outH) const;
	Vector2f WorldToScreenPoint(const Vector2f& world) const;
	Rectf WorldToScreenRect(const Vector2f& worldTopLeft, const Vector2f& size) const;

	void UpdateViewportCache() const;
	void ApplyViewportAndClip() const;
	void EnsureViewportAndClipApplied() const;

private:
	void* m_renderer = nullptr; // native* internally
	void* m_window = nullptr;   // native window* (for pixel size queries)

	//Virtual resolution settings (0 means disabled) ---
	int m_virtualW = 0;
	int m_virtualH = 0;
	ViewportScaleMode m_scaleMode = ViewportScaleMode::Letterbox;
	bool m_integerScale = false;

	// Colors
	Vector4i m_clearColor = Vector4i(0, 0, 0, 255);
	Vector4i m_letterboxColor = Vector4i(0, 0, 0, 255);

	//Cached viewport data
	mutable bool m_cacheValid = false;
	mutable int m_cachedOutputW = -1;
	mutable int m_cachedOutputH = -1;
	mutable int m_cachedGameW = 0;
	mutable int m_cachedGameH = 0;
	mutable float m_cachedScaleX = 1.0f;
	mutable float m_cachedScaleY = 1.0f;
	mutable Vector2f m_cachedOffset = Vector2f(0.0f, 0.0f); // screen-space offset (used for Stretch/Crop)
	mutable Rectf m_cachedViewport{}; // real window pixel rect (Letterbox: game area, Stretch/Crop: full window)

	// Tracks whether viewport/clip has been applied since the last clear.
	mutable bool m_viewportAppliedThisFrame = false;
};

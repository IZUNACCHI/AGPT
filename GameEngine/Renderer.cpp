#include "Renderer.h"
#include "Texture.h"
#include <SDL3/SDL.h>
#include <string>
#include <cmath>
#include <algorithm>

static SDL_Renderer* R(void* p) { return static_cast<SDL_Renderer*>(p); }

Renderer::Renderer(Window& window)
	: m_renderer(nullptr) {

	LOG_INFO("Initializing renderer");

	SDL_Window* sdlWindow = static_cast<SDL_Window*>(window.GetNative());
	if (!sdlWindow) {
		THROW_ENGINE_EXCEPTION("Cannot create renderer - window is not valid");
	}

	m_renderer = (void*)SDL_CreateRenderer(sdlWindow, nullptr);
	if (!m_renderer) {
		THROW_ENGINE_EXCEPTION("Failed to create renderer: ") << SDL_GetError();
	}

	LOG_INFO("Renderer initialized successfully");
}

Renderer::Renderer(Renderer&& other) noexcept
	: m_renderer(other.m_renderer),
	  m_virtualW(other.m_virtualW),
	  m_virtualH(other.m_virtualH),
	  m_letterbox(other.m_letterbox),
	  m_integerScale(other.m_integerScale),
	  m_clearColor(other.m_clearColor),
	  m_letterboxColor(other.m_letterboxColor),
	  m_cacheValid(false) {

	other.m_renderer = nullptr;
}

Renderer& Renderer::operator=(Renderer&& other) noexcept {
	if (this != &other) {
		if (m_renderer) {
			SDL_DestroyRenderer(static_cast<SDL_Renderer*>(m_renderer));
		}
		m_renderer = other.m_renderer;
		other.m_renderer = nullptr;

		m_virtualW = other.m_virtualW;
		m_virtualH = other.m_virtualH;
		m_letterbox = other.m_letterbox;
		m_integerScale = other.m_integerScale;
		m_clearColor = other.m_clearColor;
		m_letterboxColor = other.m_letterboxColor;

		m_cacheValid = false;
	}
	return *this;
}

Renderer::~Renderer() {
	if (m_renderer) {
		SDL_DestroyRenderer(static_cast<SDL_Renderer*>(m_renderer));
		LOG_INFO("Renderer destroyed");
	}
}

void Renderer::SetClearColor(const Vector4i& rgba) {
	m_clearColor = rgba;
}

void Renderer::SetLetterboxColor(const Vector4i& rgba) {
	m_letterboxColor = rgba;
}

void Renderer::Clear() {
	if (!m_renderer) return;

	// 1) Clear the entire window (including letterbox bars) to letterbox color.
	// We intentionally clear with no viewport/clip so the bars are clean.
	SDL_SetRenderViewport(R(m_renderer), nullptr);
	SDL_SetRenderClipRect(R(m_renderer), nullptr);
	SDL_SetRenderDrawColor(
		R(m_renderer),
		(Uint8)m_letterboxColor.x,
		(Uint8)m_letterboxColor.y,
		(Uint8)m_letterboxColor.z,
		(Uint8)m_letterboxColor.w
	);
	SDL_RenderClear(R(m_renderer));

	// 2) Apply the virtual-resolution viewport for subsequent draws,
	// then clear the game area to the clear color.
	ApplyViewportAndClip();
	SDL_SetRenderDrawColor(
		R(m_renderer),
		(Uint8)m_clearColor.x,
		(Uint8)m_clearColor.y,
		(Uint8)m_clearColor.z,
		(Uint8)m_clearColor.w
	);
	SDL_RenderClear(R(m_renderer));
}

void Renderer::Present() {
	if (!m_renderer) return;
	SDL_RenderPresent(R(m_renderer));
}

void* Renderer::GetNative() const {
	return m_renderer;
}

bool Renderer::GetOutputSize(int& outW, int& outH) const {
	outW = 0; outH = 0;
	if (!m_renderer) return false;
	return SDL_GetRenderOutputSize(R(m_renderer), &outW, &outH);
}

void Renderer::SetVirtualResolution(int width, int height) {
	m_virtualW = std::max(0, width);
	m_virtualH = std::max(0, height);
	m_cacheValid = false;
}

void Renderer::SetVirtualResolution(const Vector2i& size) {
	SetVirtualResolution(size.x, size.y);
}

Vector2i Renderer::GetVirtualResolution() const {
	return Vector2i(m_virtualW, m_virtualH);
}

void Renderer::SetLetterbox(bool enabled) {
	m_letterbox = enabled;
	m_cacheValid = false;
}

void Renderer::SetIntegerScaling(bool enabled) {
	m_integerScale = enabled;
	m_cacheValid = false;
}

Rectf Renderer::GetViewportRect() const {
	UpdateViewportCache();
	return m_cachedViewport;
}

void Renderer::UpdateViewportCache() const {
	if (!m_renderer) {
		m_cacheValid = false;
		return;
	}

	int outW = 0, outH = 0;
	if (!GetOutputSize(outW, outH)) {
		m_cacheValid = false;
		return;
	}

	if (m_cacheValid && outW == m_cachedOutputW && outH == m_cachedOutputH) {
		return;
	}

	m_cachedOutputW = outW;
	m_cachedOutputH = outH;

	// If no virtual resolution is configured, behave like before.
	if (m_virtualW <= 0 || m_virtualH <= 0) {
		m_cachedGameW = outW;
		m_cachedGameH = outH;
		m_cachedScale = 1.0f;
		m_cachedViewport = Rectf(0.0f, 0.0f, (float)outW, (float)outH);
		m_cacheValid = true;
		return;
	}

	m_cachedGameW = m_virtualW;
	m_cachedGameH = m_virtualH;

	const float scaleX = (float)outW / (float)m_virtualW;
	const float scaleY = (float)outH / (float)m_virtualH;

	// NOTE:
	// - Letterbox: pick the smaller scale so the virtual area fits fully.
	// - Stretch: pick the larger scale so it fills (cropping may happen).
	float desiredScale = m_letterbox ? std::min(scaleX, scaleY) : std::max(scaleX, scaleY);

	// Optional pixel-art friendly integer scaling.
	if (m_integerScale && desiredScale >= 1.0f) {
		desiredScale = std::floor(desiredScale);
		if (desiredScale < 1.0f) desiredScale = 1.0f;
	}

	// Compute viewport size safely.
	// Using round here can sometimes overshoot by 1px depending on floating error,
	// which can create negative offsets and "cut" the top/side bars.
	// So we:
	// 1) compute a candidate size
	// 2) clamp to output
	// 3) recompute an exact uniform scale that fits
	int vpW = (int)std::round((float)m_virtualW * desiredScale);
	int vpH = (int)std::round((float)m_virtualH * desiredScale);

	vpW = std::min(vpW, outW);
	vpH = std::min(vpH, outH);

	float fitScaleX = (float)vpW / (float)m_virtualW;
	float fitScaleY = (float)vpH / (float)m_virtualH;
	float scale = m_letterbox ? std::min(fitScaleX, fitScaleY) : std::max(fitScaleX, fitScaleY);

	// Final integer viewport based on the final scale (floor to guarantee fit).
	vpW = (int)std::floor((float)m_virtualW * scale);
	vpH = (int)std::floor((float)m_virtualH * scale);

	// Clamp again just to be safe.
	vpW = std::min(vpW, outW);
	vpH = std::min(vpH, outH);

	const int vpX = std::max(0, (outW - vpW) / 2);
	const int vpY = std::max(0, (outH - vpH) / 2);

	// Recompute scale based on the final integer viewport size.
	if (m_virtualW > 0 && m_virtualH > 0) {
		scale = std::min((float)vpW / (float)m_virtualW, (float)vpH / (float)m_virtualH);
	}
	else {
		scale = 1.0f;
	}

	m_cachedScale = scale;
	m_cachedViewport = Rectf((float)vpX, (float)vpY, (float)vpW, (float)vpH);
	m_cacheValid = true;
}

void Renderer::ApplyViewportAndClip() const {
	if (!m_renderer) return;
	UpdateViewportCache();
	if (!m_cacheValid) return;

	SDL_Rect vp{};
	vp.x = (int)std::floor(m_cachedViewport.x);
	vp.y = (int)std::floor(m_cachedViewport.y);
	vp.w = (int)std::floor(m_cachedViewport.width);
	vp.h = (int)std::floor(m_cachedViewport.height);

	// IMPORTANT:
	// SDL's viewport changes the renderer coordinate origin to the viewport's
	// top-left. That means all subsequent draw coordinates should be expressed
	// in *viewport space* (0..vp.w, 0..vp.h), NOT window space.
	//
	// Therefore:
	// - We set the viewport in window space (vp.x/y are window pixels)
	// - We set the clip rect in viewport space (0,0..vp.w/vp.h)
	SDL_SetRenderViewport(R(m_renderer), &vp);
	SDL_Rect clip{ 0, 0, vp.w, vp.h };
	SDL_SetRenderClipRect(R(m_renderer), &clip);
}

Vector2f Renderer::WorldToScreenPoint(const Vector2f& world) const {
	UpdateViewportCache();
	if (!m_cacheValid) return world; // fallback

	// WORLD: origin center, +Y up, in VIRTUAL pixels.
	// VIRTUAL SCREEN: origin top-left, +Y down, size = (m_cachedGameW, m_cachedGameH)
	// VIEWPORT SPACE: apply uniform scale. The viewport offset is applied
	// by SDL_SetRenderViewport in ApplyViewportAndClip().
	const float vx = world.x + (float)m_cachedGameW * 0.5f;
	const float vy = (float)m_cachedGameH * 0.5f - world.y;

	return Vector2f(
		vx * m_cachedScale,
		vy * m_cachedScale
	);
}

Rectf Renderer::WorldToScreenRect(const Vector2f& worldTopLeft, const Vector2f& size) const {
	const Vector2f screenTL = WorldToScreenPoint(worldTopLeft);
	UpdateViewportCache();
	const float s = m_cacheValid ? m_cachedScale : 1.0f;
	return Rectf(screenTL.x, screenTL.y, size.x * s, size.y * s);
}

static SDL_FlipMode ToSDLFlip(FlipMode flip) {
	switch (flip) {
	case FlipMode::None: return SDL_FLIP_NONE;
	case FlipMode::Horizontal: return SDL_FLIP_HORIZONTAL;
	case FlipMode::Vertical: return SDL_FLIP_VERTICAL;
	case FlipMode::Both: return (SDL_FlipMode)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	default: return SDL_FLIP_NONE;
	}
}

bool Renderer::DrawTexture(
	const Texture& texture,
	const Vector2f& sourcePosition,
	const Vector2f& sourceSize,
	const Vector2f& destinationPosition,
	const Vector2f& destinationSize) {

	if (!m_renderer) return false;
	if (!texture.IsValid()) return false;
	ApplyViewportAndClip();

	const SDL_FRect src{ sourcePosition.x, sourcePosition.y, sourceSize.x, sourceSize.y };
	const Rectf dstR = WorldToScreenRect(destinationPosition, destinationSize);
	const SDL_FRect dst{ dstR.x, dstR.y, dstR.width, dstR.height };

	SDL_Texture* sdlTex = static_cast<SDL_Texture*>(texture.GetNative());
	if (!sdlTex) return false;

	if (!SDL_RenderTexture(R(m_renderer), sdlTex, &src, &dst)) {
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
	FlipMode flip) {

	if (!m_renderer) return false;
	if (!texture.IsValid()) return false;
	ApplyViewportAndClip();

	const SDL_FRect src{ sourcePosition.x, sourcePosition.y, sourceSize.x, sourceSize.y };
	const Rectf dstR = WorldToScreenRect(destinationPosition, destinationSize);
	const SDL_FRect dst{ dstR.x, dstR.y, dstR.width, dstR.height };

	SDL_Texture* sdlTex = static_cast<SDL_Texture*>(texture.GetNative());
	if (!sdlTex) return false;

	SDL_FPoint center{};
	if (pivot.x < 0.0f || pivot.y < 0.0f) {
		center.x = dst.w * 0.5f;
		center.y = dst.h * 0.5f;
	}
	else {
		UpdateViewportCache();
		const float s = m_cacheValid ? m_cachedScale : 1.0f;
		center.x = pivot.x * s;
		center.y = pivot.y * s;
	}

	// Preserve WORLD CCW meaning with Y-down screen
	const float sdlAngle = -angleDegrees;

	if (!SDL_RenderTextureRotated(R(m_renderer), sdlTex, &src, &dst, sdlAngle, &center, ToSDLFlip(flip))) {
		LOG_WARN("Renderer draw rotated texture failed: " + std::string(SDL_GetError()));
		return false;
	}
	return true;
}

bool Renderer::DrawRectOutline(const Vector2f& worldTopLeft, const Vector2f& size, const Vector3i& color) {
	if (!m_renderer) return false;
	ApplyViewportAndClip();

	if (!SDL_SetRenderDrawColor(R(m_renderer), (Uint8)color.x, (Uint8)color.y, (Uint8)color.z, 255)) {
		LOG_WARN("Renderer draw rect failed to set color: " + std::string(SDL_GetError()));
		return false;
	}

	const Rectf r = WorldToScreenRect(worldTopLeft, size);
	const SDL_FRect rect{ r.x, r.y, r.width, r.height };

	if (!SDL_RenderRect(R(m_renderer), &rect)) {
		LOG_WARN("Renderer draw rect failed: " + std::string(SDL_GetError()));
		return false;
	}

	return true;
}

bool Renderer::DrawCircleOutline(const Vector2f& worldCenter, float radius, const Vector3i& color, int segments) {
	if (!m_renderer) return false;
	if (radius <= 0.0f || segments < 3) return false;
	ApplyViewportAndClip();

	if (!SDL_SetRenderDrawColor(R(m_renderer), (Uint8)color.x, (Uint8)color.y, (Uint8)color.z, 255)) {
		LOG_WARN("Renderer draw circle failed to set color: " + std::string(SDL_GetError()));
		return false;
	}

	const Vector2f c = WorldToScreenPoint(worldCenter);
	UpdateViewportCache();
	const float s = m_cacheValid ? m_cachedScale : 1.0f;
	const float rpx = radius * s;

	const float step = Math::Constants<float>::TwoPi / (float)segments;
	float a = 0.0f;

	float prevX = c.x + rpx;
	float prevY = c.y;

	for (int i = 1; i <= segments; ++i) {
		a += step;

		// screen y is down, so use -sin to match world +Y up
		const float x = c.x + std::cos(a) * rpx;
		const float y = c.y - std::sin(a) * rpx;

		SDL_RenderLine(R(m_renderer), prevX, prevY, x, y);
		prevX = x;
		prevY = y;
	}

	return true;
}

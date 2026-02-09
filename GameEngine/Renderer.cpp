#include "Renderer.h"
#include "Texture.h"
#include <SDL3/SDL.h>
#include <string>
#include <cmath>
#include <algorithm>

static SDL_Renderer* R(void* p) { return static_cast<SDL_Renderer*>(p); }
static SDL_Window* W(void* p) { return static_cast<SDL_Window*>(p); }

Renderer::Renderer(Window& window)
	: m_renderer(nullptr) {

	LOG_INFO("Initializing renderer");

	SDL_Window* sdlWindow = static_cast<SDL_Window*>(window.GetNative());
	if (!sdlWindow) {
		THROW_ENGINE_EXCEPTION("Cannot create renderer - window is not valid");
	}

	// Store native window handle so we can query pixel size reliably
	// across resizes / fullscreen changes / DPI scaling.
	m_window = (void*)sdlWindow;

	m_renderer = (void*)SDL_CreateRenderer(sdlWindow, nullptr);
	if (!m_renderer) {
		THROW_ENGINE_EXCEPTION("Failed to create renderer: ") << SDL_GetError();
	}

	LOG_INFO("Renderer initialized successfully");
}

Renderer::Renderer(Renderer&& other) noexcept
	: m_renderer(other.m_renderer),
	m_window(other.m_window),
	m_virtualW(other.m_virtualW),
	m_virtualH(other.m_virtualH),
	m_scaleMode(other.m_scaleMode),
	m_integerScale(other.m_integerScale),
	m_clearColor(other.m_clearColor),
	m_letterboxColor(other.m_letterboxColor),
	m_cacheValid(false) {

	other.m_renderer = nullptr;
	other.m_window = nullptr;
}

Renderer& Renderer::operator=(Renderer&& other) noexcept {
	if (this != &other) {
		if (m_renderer) {
			SDL_DestroyRenderer(static_cast<SDL_Renderer*>(m_renderer));
		}
		m_renderer = other.m_renderer;
		m_window = other.m_window;
		other.m_renderer = nullptr;
		other.m_window = nullptr;

		m_virtualW = other.m_virtualW;
		m_virtualH = other.m_virtualH;
		m_scaleMode = other.m_scaleMode;
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

	// New frame: viewport/clip not applied yet.
	m_viewportAppliedThisFrame = false;

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
	m_viewportAppliedThisFrame = true;
	SDL_SetRenderDrawColor(
		R(m_renderer),
		(Uint8)m_clearColor.x,
		(Uint8)m_clearColor.y,
		(Uint8)m_clearColor.z,
		(Uint8)m_clearColor.w
	);
	SDL_RenderClear(R(m_renderer));
}

void Renderer::BeginFrame() {
	Clear();
}

void Renderer::Present() {
	if (!m_renderer) return;
	SDL_RenderPresent(R(m_renderer));
	// After presenting, consider the frame closed.
	m_viewportAppliedThisFrame = false;
}

void Renderer::EndFrame() {
	Present();
}

void* Renderer::GetNative() const {
	return m_renderer;
}

bool Renderer::GetOutputSize(int& outW, int& outH) const {
	outW = 0; outH = 0;
	if (!m_renderer) return false;

	// Prefer the window pixel size. This updates correctly when switching
	// fullscreen / borderless / resizing at runtime and accounts for DPI.
	if (m_window) {
		SDL_GetWindowSizeInPixels(W(m_window), &outW, &outH);
		return (outW > 0 && outH > 0);
	}

	// Fallback: renderer output size.
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
	SetViewportScaleMode(enabled ? ViewportScaleMode::Letterbox : ViewportScaleMode::Crop);
}

void Renderer::SetViewportScaleMode(ViewportScaleMode mode) {
	m_scaleMode = mode;
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
		m_cachedScaleX = 1.0f;
		m_cachedScaleY = 1.0f;
		m_cachedOffset = Vector2f(0.0f, 0.0f);
		m_cachedViewport = Rectf(0.0f, 0.0f, (float)outW, (float)outH);
		m_cacheValid = true;
		return;
	}

	m_cachedGameW = m_virtualW;
	m_cachedGameH = m_virtualH;

	const float rawScaleX = (float)outW / (float)m_virtualW;
	const float rawScaleY = (float)outH / (float)m_virtualH;

	// Default: full-window viewport, no offset
	m_cachedViewport = Rectf(0.0f, 0.0f, (float)outW, (float)outH);
	m_cachedOffset = Vector2f(0.0f, 0.0f);

	if (m_scaleMode == ViewportScaleMode::Stretch) {
		// Fill the whole window, distort aspect.
		m_cachedScaleX = rawScaleX;
		m_cachedScaleY = rawScaleY;
	}
	else {
		// Uniform-scale modes (Letterbox/Crop)
		float uniform = (m_scaleMode == ViewportScaleMode::Letterbox)
			? std::min(rawScaleX, rawScaleY)
			: std::max(rawScaleX, rawScaleY);

		// Optional pixel-art friendly integer scaling (only really makes sense for uniform).
		if (m_integerScale && uniform >= 1.0f) {
			uniform = std::floor(uniform);
			if (uniform < 1.0f) uniform = 1.0f;
		}

		// Compute scaled virtual size.
		// Use floor for Letterbox to guarantee it fits.
		// For Crop, it can exceed the window.
		int scaledW = (m_scaleMode == ViewportScaleMode::Letterbox)
			? (int)std::floor((float)m_virtualW * uniform)
			: (int)std::ceil((float)m_virtualW * uniform);
		int scaledH = (m_scaleMode == ViewportScaleMode::Letterbox)
			? (int)std::floor((float)m_virtualH * uniform)
			: (int)std::ceil((float)m_virtualH * uniform);

		// Avoid 0 sizes.
		scaledW = std::max(1, scaledW);
		scaledH = std::max(1, scaledH);

		if (m_scaleMode == ViewportScaleMode::Letterbox) {
			// Center the game area inside the window, bars around it.
			scaledW = std::min(scaledW, outW);
			scaledH = std::min(scaledH, outH);
			const int vpX = (outW - scaledW) / 2;
			const int vpY = (outH - scaledH) / 2;
			m_cachedViewport = Rectf((float)vpX, (float)vpY, (float)scaledW, (float)scaledH);

			// In Letterbox mode we render in viewport-local space, so offset is 0.
			m_cachedOffset = Vector2f(0.0f, 0.0f);
		}
		else {
			// Crop: viewport is full window, but we offset the content so it's centered.
			// Offset can be negative (content larger than window).
			const float offX = ((float)outW - (float)scaledW) * 0.5f;
			const float offY = ((float)outH - (float)scaledH) * 0.5f;
			m_cachedOffset = Vector2f(offX, offY);
		}

		m_cachedScaleX = uniform;
		m_cachedScaleY = uniform;
	}

	m_cacheValid = true;
}

void Renderer::ApplyViewportAndClip() const {
	if (!m_renderer) return;
	UpdateViewportCache();
	if (!m_cacheValid) return;

	if (m_scaleMode == ViewportScaleMode::Letterbox && m_virtualW > 0 && m_virtualH > 0) {
		SDL_Rect vp{};
		vp.x = (int)std::floor(m_cachedViewport.x);
		vp.y = (int)std::floor(m_cachedViewport.y);
		vp.w = (int)std::floor(m_cachedViewport.width);
		vp.h = (int)std::floor(m_cachedViewport.height);

		// In Letterbox mode, we use an SDL viewport for the bars.
		SDL_SetRenderViewport(R(m_renderer), &vp);
		SDL_Rect clip{ 0, 0, vp.w, vp.h };
		SDL_SetRenderClipRect(R(m_renderer), &clip);
	}
	else {
		// Stretch/Crop (or no virtual resolution): use full window.
		SDL_SetRenderViewport(R(m_renderer), nullptr);
		SDL_SetRenderClipRect(R(m_renderer), nullptr);
	}
}

void Renderer::EnsureViewportAndClipApplied() const {
	if (m_viewportAppliedThisFrame) {
		return;
	}
	ApplyViewportAndClip();
	m_viewportAppliedThisFrame = true;
}

Vector2f Renderer::WorldToScreenPoint(const Vector2f& world) const {
	UpdateViewportCache();
	if (!m_cacheValid) return world; // fallback

	// WORLD: origin center, +Y up, in VIRTUAL pixels.
	// VIRTUAL SCREEN: origin top-left, +Y down, size = (m_cachedGameW, m_cachedGameH)
	// VIEWPORT SPACE (Letterbox): apply scale, SDL viewport applies the offset.
	// WINDOW SPACE (Stretch/Crop): apply scale + cached offset.
	const float vx = world.x + (float)m_cachedGameW * 0.5f;
	const float vy = (float)m_cachedGameH * 0.5f - world.y;

	return Vector2f(
		m_cachedOffset.x + vx * m_cachedScaleX,
		m_cachedOffset.y + vy * m_cachedScaleY
	);
}

Rectf Renderer::WorldToScreenRect(const Vector2f& worldTopLeft, const Vector2f& size) const {
	const Vector2f screenTL = WorldToScreenPoint(worldTopLeft);
	UpdateViewportCache();
	const float sx = m_cacheValid ? m_cachedScaleX : 1.0f;
	const float sy = m_cacheValid ? m_cachedScaleY : 1.0f;
	return Rectf(screenTL.x, screenTL.y, size.x * sx, size.y * sy);
}

static SDL_FlipMode ToSDLFlip(FlipMode flip) {
	switch (flip) {
	case FlipMode::None: return SDL_FLIP_NONE;
	case FlipMode::Horizontal: return SDL_FLIP_HORIZONTAL;
	case FlipMode::Vertical: return SDL_FLIP_VERTICAL;
	case FlipMode::Both: return static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
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
	EnsureViewportAndClipApplied();

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

bool Renderer::DrawTextureTinted(
	const Texture& texture,
	const Vector2f& sourcePosition,
	const Vector2f& sourceSize,
	const Vector2f& destinationPosition,
	const Vector2f& destinationSize,
	const Vector4i& tint) {

	if (!m_renderer) return false;
	if (!texture.IsValid()) return false;
	EnsureViewportAndClipApplied();

	SDL_Texture* sdlTex = static_cast<SDL_Texture*>(texture.GetNative());
	if (!sdlTex) return false;

	// Save current modulation so this is safe for shared textures.
	Uint8 oldR = 255, oldG = 255, oldB = 255, oldA = 255;
	SDL_GetTextureColorMod(sdlTex, &oldR, &oldG, &oldB);
	SDL_GetTextureAlphaMod(sdlTex, &oldA);

	SDL_SetTextureColorMod(sdlTex, (Uint8)tint.x, (Uint8)tint.y, (Uint8)tint.z);
	SDL_SetTextureAlphaMod(sdlTex, (Uint8)tint.w);

	const SDL_FRect src{ sourcePosition.x, sourcePosition.y, sourceSize.x, sourceSize.y };
	const Rectf dstR = WorldToScreenRect(destinationPosition, destinationSize);
	const SDL_FRect dst{ dstR.x, dstR.y, dstR.width, dstR.height };

	bool ok = SDL_RenderTexture(R(m_renderer), sdlTex, &src, &dst);
	if (!ok) {
		LOG_WARN("Renderer draw tinted texture failed: " + std::string(SDL_GetError()));
	}

	// Restore
	SDL_SetTextureColorMod(sdlTex, oldR, oldG, oldB);
	SDL_SetTextureAlphaMod(sdlTex, oldA);

	return ok;
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
	EnsureViewportAndClipApplied();

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
		const float sx = m_cacheValid ? m_cachedScaleX : 1.0f;
		const float sy = m_cacheValid ? m_cachedScaleY : 1.0f;
		center.x = pivot.x * sx;
		center.y = pivot.y * sy;
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
	EnsureViewportAndClipApplied();

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

bool Renderer::DrawRectOutlineRotated(const Vector2f& worldCenter, const Vector2f& size, float angleDegrees, const Vector3i& color) {
	if (!m_renderer) return false;
	EnsureViewportAndClipApplied();

	if (!SDL_SetRenderDrawColor(R(m_renderer), (Uint8)color.x, (Uint8)color.y, (Uint8)color.z, 255)) {
		LOG_WARN("Renderer draw rect failed to set color: " + std::string(SDL_GetError()));
		return false;
	}

	const float radians = angleDegrees * Math::Constants<float>::Deg2Rad;
	const float cosA = std::cos(radians);
	const float sinA = std::sin(radians);
	const Vector2f half(size.x * 0.5f, size.y * 0.5f);

	const Vector2f corners[4] = {
		Vector2f(-half.x, -half.y),
		Vector2f(half.x, -half.y),
		Vector2f(half.x, half.y),
		Vector2f(-half.x, half.y)
	};

	SDL_FPoint points[5];
	for (int i = 0; i < 4; ++i) {
		const Vector2f local = corners[i];
		const Vector2f rotated(
			local.x * cosA - local.y * sinA,
			local.x * sinA + local.y * cosA
		);
		const Vector2f world = worldCenter + rotated;
		const Vector2f screen = WorldToScreenPoint(world);
		points[i] = SDL_FPoint{ screen.x, screen.y };
	}
	points[4] = points[0];

	if (!SDL_RenderLines(R(m_renderer), points, 5)) {
		LOG_WARN("Renderer draw rect failed: " + std::string(SDL_GetError()));
		return false;
	}

	return true;
}

bool Renderer::DrawFilledRect(const Vector2f& worldTopLeft, const Vector2f& size, const Vector4i& color) {
	if (!m_renderer) return false;
	EnsureViewportAndClipApplied();

	SDL_SetRenderDrawBlendMode(R(m_renderer), SDL_BLENDMODE_BLEND);
	if (!SDL_SetRenderDrawColor(R(m_renderer), (Uint8)color.x, (Uint8)color.y, (Uint8)color.z, (Uint8)color.w)) {
		LOG_WARN("Renderer fill rect failed to set color: " + std::string(SDL_GetError()));
		return false;
	}

	const Rectf r = WorldToScreenRect(worldTopLeft, size);
	const SDL_FRect dst{ r.x, r.y, r.width, r.height };

	if (!SDL_RenderFillRect(R(m_renderer), &dst)) {
		LOG_WARN("Renderer fill rect failed: " + std::string(SDL_GetError()));
		return false;
	}
	return true;
}

bool Renderer::DrawCircleOutline(const Vector2f& worldCenter, float radius, const Vector3i& color, int segments) {
	if (!m_renderer) return false;
	if (radius <= 0.0f || segments < 3) return false;
	EnsureViewportAndClipApplied();

	if (!SDL_SetRenderDrawColor(R(m_renderer), (Uint8)color.x, (Uint8)color.y, (Uint8)color.z, 255)) {
		LOG_WARN("Renderer draw circle failed to set color: " + std::string(SDL_GetError()));
		return false;
	}

	const Vector2f c = WorldToScreenPoint(worldCenter);
	UpdateViewportCache();
	// For Stretch, circles become ellipses. We approximate using the average scale.
	const float sx = m_cacheValid ? m_cachedScaleX : 1.0f;
	const float sy = m_cacheValid ? m_cachedScaleY : 1.0f;
	const float rpx = radius * ((sx + sy) * 0.5f);

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

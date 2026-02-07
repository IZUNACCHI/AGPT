#pragma once

#include "UIElement.h"
#include "Types.hpp"

class Texture;
class Renderer;

/// Minimal textured UI element.
/// Draws a texture (optionally a source sub-rect) into the UI element rect.
class UIImage : public UIElement {
public:
	UIImage();
	~UIImage() override = default;

	void SetTexture(Texture* tex) { m_texture = tex; }
	Texture* GetTexture() const { return m_texture; }

	// Source rectangle in pixels inside the texture.
	void SetSourceRect(const Vector2f& srcPos, const Vector2f& srcSize) {
		m_srcPos = srcPos;
		m_srcSize = srcSize;
		m_hasSource = true;
	}
	void ClearSourceRect() { m_hasSource = false; }

	void Render(Renderer& renderer) override;

private:
	Texture* m_texture = nullptr; // not owned
	bool m_hasSource = false;
	Vector2f m_srcPos{ 0.0f, 0.0f };
	Vector2f m_srcSize{ 0.0f, 0.0f };
};

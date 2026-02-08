#pragma once

#include "UIElement.h"
#include "Types.hpp"

#include <string>

class BitmapFont;
class Renderer;

enum class UILabelAnchor {
	TopLeft,
	TopCenter,
	Center
};

// Simple non-interactive text element for the UI system.
// Uses BitmapFont (loaded via AssetManager / LoadFont).
class UILabel : public UIElement {
public:
	UILabel();
	~UILabel() override = default;

	void SetFont(BitmapFont* font) { m_font = font; }
	BitmapFont* GetFont() const { return m_font; }

	void SetText(const std::string& text) { m_text = text; }
	const std::string& GetText() const { return m_text; }

	void SetColor(const Vector4i& rgba) { m_color = rgba; }
	Vector4i GetColor() const { return m_color; }

	void SetScale(const Vector2f& scale) { m_scale = scale; }
	Vector2f GetScale() const { return m_scale; }

	void SetAnchor(UILabelAnchor anchor) { m_anchor = anchor; }
	UILabelAnchor GetAnchor() const { return m_anchor; }

	void Render(Renderer& renderer) override;

private:
	BitmapFont* m_font = nullptr; // not owned
	std::string m_text;
	Vector4i m_color{ 255, 255, 255, 255 };
	Vector2f m_scale{ 1.0f, 1.0f };
	UILabelAnchor m_anchor = UILabelAnchor::TopLeft;
};

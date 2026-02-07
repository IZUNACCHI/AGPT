#pragma once

#include "RenderableComponent.h"
#include "Types.hpp"
#include <memory>
#include <string>
class Renderer;
class BitmapFont;

enum class TextAnchor {
	TopLeft,
	Center
};

// Uses Transform:
// position = anchor in WORLD
// rotation = rotates entire block
// scale = scales block (negative scale mirrors)
class TextRenderer : public RenderableComponent {
public:
	TextRenderer();

	void SetFont(BitmapFont* font) { m_font = font; }
	void SetText(const std::string& text) { m_text = text; }
	void SetAnchor(TextAnchor a) { m_anchor = a; }
	void SetExtraScale(float s) { m_extraScale = s; } // multiplier on top of Transform scale
	void SetLayerOrder(int order) { m_layerOrder = order; }
	int GetLayerOrder() const { return m_layerOrder; }

	static void RenderAll(Renderer& renderer);
	void Render(Renderer& renderer) const;

	std::shared_ptr<Component> Clone() const override;

private:
	static Vector2f RotateDeg(const Vector2f& v, float deg);

private:
	BitmapFont* m_font = nullptr;
	std::string m_text = "Text";

	TextAnchor m_anchor = TextAnchor::Center;
	float m_extraScale = 1.0f;
	int m_layerOrder = 0;
};

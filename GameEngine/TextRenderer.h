#pragma once

#include "Component.h"
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
class TextRenderer : public Component {
public:
	TextRenderer();

	void SetFont(const std::shared_ptr<BitmapFont>& font) { m_font = font; }
	void SetText(const std::string& text) { m_text = text; }
	void SetAnchor(TextAnchor a) { m_anchor = a; }
	void SetExtraScale(float s) { m_extraScale = s; } // multiplier on top of Transform scale
	void SetLayerOrder(int order) { m_layerOrder = order; }

	static void RenderAll(Renderer& renderer);
	void Render(Renderer& renderer) const;

	std::shared_ptr<Component> Clone() const override;

private:
	static Vector2f RotateDeg(const Vector2f& v, float deg);

private:
	std::shared_ptr<BitmapFont> m_font;
	std::string m_text = "Text";

	TextAnchor m_anchor = TextAnchor::Center;
	float m_extraScale = 1.0f;
	int m_layerOrder = 0;
};

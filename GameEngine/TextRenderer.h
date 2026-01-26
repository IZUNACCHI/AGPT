#pragma once

#include "Component.h"
#include "Types.hpp"
#include <memory>
#include <string>

class Renderer;
class BitmapFont;

enum class TextAnchor {
	TopLeft,  // Transform position is text top-left (before rotation)
	Center    // Transform position is text center (before rotation)
};

class TextRenderer : public Component {
public:
	TextRenderer();

	void SetFont(const std::shared_ptr<BitmapFont>& font) { m_font = font; }
	void SetText(const std::string& text) { m_text = text; }

	// Extra multiplier on top of Transform scale (uniform).
	void SetScale(float s) { m_extraScale = s; }

	void SetAnchor(TextAnchor a) { m_anchor = a; }
	void SetLayerOrder(int order) { m_layerOrder = order; }

	static void RenderAll(Renderer& renderer);
	void Render(Renderer& renderer) const;

	std::shared_ptr<Component> Clone() const override;

private:
	std::shared_ptr<BitmapFont> m_font;
	std::string m_text = "Text";

	float m_extraScale = 1.0f;
	TextAnchor m_anchor = TextAnchor::Center;
	int m_layerOrder = 0;
};

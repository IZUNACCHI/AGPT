#pragma once

#include "UIElement.h"

class Renderer;

struct UIPanelStyle {
	// RGBA (alpha supported)
	Vector4i color{ 0, 0, 0, 128 };
	// Border (alpha ignored by Renderer::DrawRectOutline)
	Vector3i borderColor{ 255, 255, 255 };
	float borderThickness = 0.0f;
	bool showBorder = false;
};

// Simple filled rectangle panel (useful for backdrops / screen dimming).
class UIPanel : public UIElement {
public:
	UIPanel();
	~UIPanel() override = default;

	void SetStyle(const UIPanelStyle& s) { m_style = s; }
	const UIPanelStyle& GetStyle() const { return m_style; }

	void Render(Renderer& renderer) override;

private:
	UIPanelStyle m_style{};
};

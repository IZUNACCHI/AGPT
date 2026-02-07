#include "UIProgressBar.h"

#include "Renderer.h"

#include <algorithm>

UIProgressBar::UIProgressBar() : UIElement() {
}

void UIProgressBar::SetValue(float v) {
	m_value = std::clamp(v, 0.0f, 1.0f);
}

void UIProgressBar::Render(Renderer& renderer) {
	const Vector2f worldTopLeft = UIToWorldTopLeft(Vector2f(m_rect.x, m_rect.y));
	const Vector2f size(m_rect.w, m_rect.h);

	// Back
	renderer.DrawFilledRect(worldTopLeft, size, m_style.backColor);

	// Fill
	float fillW = size.x * m_value;
	if (fillW > 0.0f) {
		Vector2f fillTopLeft = worldTopLeft;
		if (!m_style.leftToRight) {
			fillTopLeft.x = worldTopLeft.x + (size.x - fillW);
		}
		renderer.DrawFilledRect(fillTopLeft, Vector2f(fillW, size.y), m_style.fillColor);
	}

	if (m_style.showBorder && m_style.borderThickness > 0.0f) {
		const Vector3i bc(m_style.borderColor.x, m_style.borderColor.y, m_style.borderColor.z);
		renderer.DrawRectOutline(worldTopLeft, size, bc);
	}
}

#include "UIPanel.h"

#include "Renderer.h"

UIPanel::UIPanel() : UIElement() {
	// Panels are usually decorative and should not steal hover/clicks unless asked.
	SetInteractable(false);
}

void UIPanel::Render(Renderer& renderer) {
	const Vector2f worldTopLeft = UIToWorldTopLeft(Vector2f(m_rect.x, m_rect.y));
	const Vector2f size(m_rect.w, m_rect.h);

	renderer.DrawFilledRect(worldTopLeft, size, m_style.color);

	if (m_style.showBorder && m_style.borderThickness > 0.0f) {
		renderer.DrawRectOutline(worldTopLeft, size, m_style.borderColor);
	}
}

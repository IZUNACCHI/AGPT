#include "UILabel.h"

#include "BitmapFont.h"
#include "Renderer.h"

UILabel::UILabel() : UIElement() {
	SetInteractable(false);
}

void UILabel::Render(Renderer& renderer) {
	if (!m_font) return;

	// Default: use rect.x/y as the anchor position in UI space.
	Vector2f uiPos((float)m_rect.x, (float)m_rect.y);
	Vector2f textSize = m_font->MeasureText(m_text, m_scale);

	switch (m_anchor) {
	case UILabelAnchor::TopLeft:
		break;
	case UILabelAnchor::TopCenter:
		uiPos.x -= textSize.x * 0.5f;
		break;
	case UILabelAnchor::Center:
		uiPos.x -= textSize.x * 0.5f;
		uiPos.y -= textSize.y * 0.5f;
		break;
	default:
		break;
	}

	Vector2f worldTopLeft = UIToWorldTopLeft(uiPos);
	m_font->DrawColored(renderer, m_text, worldTopLeft, m_scale, m_color);
}

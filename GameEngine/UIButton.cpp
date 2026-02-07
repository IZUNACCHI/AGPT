#include "UIButton.h"

#include "BitmapFont.h"
#include "Renderer.h"

UIButton::UIButton() : UISelectable() {
}

void UIButton::OnPointerEnter() {
	m_hovered = true;
}

void UIButton::OnPointerExit() {
	m_hovered = false;
	m_pressed = false;
}

void UIButton::OnPointerDown() {
	m_pressed = true;
}

void UIButton::OnPointerUp() {
	m_pressed = false;
}

void UIButton::OnSubmit() {
	if (!IsInteractable()) return;
	if (m_onClick) m_onClick();
}

Vector4i UIButton::GetBackgroundColor() const {
	if (!IsInteractable()) return m_style.bgDisabled;
	if (m_pressed) return m_style.bgPressed;
	if (m_hovered) return m_style.bgHover;
	if (IsFocused()) return m_style.bgFocused;
	return m_style.bgNormal;
}

void UIButton::Render(Renderer& renderer) {
	const Vector2f worldTopLeft = UIToWorldTopLeft(Vector2f(m_rect.x, m_rect.y));
	const Vector2f size(m_rect.w, m_rect.h);

	// Background
	renderer.DrawFilledRect(worldTopLeft, size, GetBackgroundColor());

	// Border (alpha ignored here; keep it simple)
	if (m_style.borderThickness > 0.0f) {
		const Vector3i bc(m_style.borderColor.x, m_style.borderColor.y, m_style.borderColor.z);
		renderer.DrawRectOutline(worldTopLeft, size, bc);
	}

	// Text
	if (!m_style.font) return;
	const Vector2f textScale(1.0f, 1.0f);
	Vector2f textSize = m_style.font->MeasureText(m_text, textScale);

	// Center text inside rect with padding clamped
	float innerW = std::max(0.0f, m_rect.w - m_style.padding * 2.0f);
	float innerH = std::max(0.0f, m_rect.h - m_style.padding * 2.0f);
	float tx = m_rect.x + m_style.padding + (innerW - textSize.x) * 0.5f;
	float ty = m_rect.y + m_style.padding + (innerH - textSize.y) * 0.5f;

	Vector2f textWorld = UIToWorldTopLeft(Vector2f(tx, ty));
	m_style.font->DrawColored(renderer, m_text, textWorld, textScale, m_style.textColor);
}

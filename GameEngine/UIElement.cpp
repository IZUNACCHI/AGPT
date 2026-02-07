#include "UIElement.h"

#include "GameObject.h"
#include "Transform.h"

#include "UICanvas.h"
#include "UISystem.h"
#include "Renderer.h"

UIElement::UIElement() : MonoBehaviour() {
}

void UIElement::Awake() {
	RefreshCanvasCache();
	UISystem::Get().RegisterElement(this);
}

void UIElement::OnDestroy() {
	UISystem::Get().UnregisterElement(this);
}

void UIElement::RefreshCanvasCache() {
	m_canvas = nullptr;
	Transform* t = GetTransform();
	while (t) {
		GameObject* go = t->GetGameObject();
		if (go) {
			if (auto canvas = go->GetComponent<UICanvas>()) {
				m_canvas = canvas.get();
				break;
			}
		}
		t = t->GetParent();
	}
}

bool UIElement::ContainsPoint(float uiX, float uiY) const {
	return uiX >= m_rect.x && uiY >= m_rect.y && uiX <= (m_rect.x + m_rect.w) && uiY <= (m_rect.y + m_rect.h);
}

Vector2f UIElement::GetCenter() const {
	return Vector2f(m_rect.x + m_rect.w * 0.5f, m_rect.y + m_rect.h * 0.5f);
}

Vector2f UIElement::UIToWorldTopLeft(const Vector2f& uiTopLeft) const {
	int uiW = 0, uiH = 0;
	UISystem::GetVirtualUISize(uiW, uiH);

	// WORLD:
	// (0,0) is center, +Y up, destination is WORLD top-left
	// UI:
	// (0,0) is top-left, +Y down
	const float worldX = -uiW * 0.5f + uiTopLeft.x;
	const float worldY =  uiH * 0.5f - uiTopLeft.y;
	return Vector2f(worldX, worldY);
}

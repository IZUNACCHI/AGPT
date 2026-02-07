#include "UISelectable.h"

#include "UISystem.h"

UISelectable::UISelectable() : UIElement() {
}

void UISelectable::Awake() {
	UIElement::Awake();
	UISystem::Get().RegisterSelectable(this);
}

void UISelectable::OnDestroy() {
	UISystem::Get().UnregisterSelectable(this);
	UIElement::OnDestroy();
}

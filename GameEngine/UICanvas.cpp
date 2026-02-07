#include "UICanvas.h"
#include "UISystem.h"

UICanvas::UICanvas() : MonoBehaviour() {
}

void UICanvas::Awake() {
	UISystem::Get().RegisterCanvas(this);
}

void UICanvas::OnDestroy() {
	UISystem::Get().UnregisterCanvas(this);
}

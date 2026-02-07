#include "UISystem.h"

#include "SleeplessEngine.h"
#include "Renderer.h"
#include "Input.h"
#include "Time.hpp"

#include "UICanvas.h"
#include "UIElement.h"
#include "UISelectable.h"

#include <algorithm>
#include <cfloat>

namespace {
	template<typename T>
	void RemovePtr(std::vector<T*>& v, T* p) {
		auto it = std::remove(v.begin(), v.end(), p);
		v.erase(it, v.end());
	}

	int ClampDir(int v) {
		if (v < -1) return -1;
		if (v > 1) return 1;
		return v;
	}
}

UISystem& UISystem::Get() {
	static UISystem instance;
	return instance;
}

void UISystem::Shutdown() {
	// Clear any stored pointers so we don't touch destroyed UI on shutdown.
	m_focused = nullptr;
	m_hovered = nullptr;
	m_mouseDown = nullptr;
	m_focusWasSetByHover = false;
	m_stickCooldown = 0.0f;

	m_selectables.clear();
	m_elements.clear();
	m_canvases.clear();
}

void UISystem::RegisterCanvas(UICanvas* canvas) {
	if (!canvas) return;
	if (std::find(m_canvases.begin(), m_canvases.end(), canvas) != m_canvases.end()) return;
	m_canvases.push_back(canvas);
}

void UISystem::UnregisterCanvas(UICanvas* canvas) {
	RemovePtr(m_canvases, canvas);
}

void UISystem::RegisterElement(UIElement* element) {
	if (!element) return;
	if (std::find(m_elements.begin(), m_elements.end(), element) != m_elements.end()) return;
	m_elements.push_back(element);
}

void UISystem::UnregisterElement(UIElement* element) {
	if (m_hovered == element) m_hovered = nullptr;
	if (m_mouseDown == element) m_mouseDown = nullptr;
	RemovePtr(m_elements, element);
}

void UISystem::RegisterSelectable(UISelectable* selectable) {
	if (!selectable) return;
	if (std::find(m_selectables.begin(), m_selectables.end(), selectable) != m_selectables.end()) return;
	m_selectables.push_back(selectable);
}

void UISystem::UnregisterSelectable(UISelectable* selectable) {
	if (m_focused == selectable) m_focused = nullptr;
	RemovePtr(m_selectables, selectable);
}

bool UISystem::GetVirtualUISize(int& outW, int& outH) {
	outW = 0; outH = 0;
	auto* r = SleeplessEngine::GetInstance().GetRenderer();
	if (!r) return false;
	Vector2i virt = r->GetVirtualResolution();
	if (virt.x > 0 && virt.y > 0) {
		outW = virt.x;
		outH = virt.y;
		return true;
	}
	// No virtual res: fall back to window pixel size.
	auto* w = SleeplessEngine::GetInstance().GetWindow();
	if (!w) return false;
	Vector2i ws = w->GetSize();
	outW = ws.x;
	outH = ws.y;
	return (outW > 0 && outH > 0);
}

Vector2f UISystem::ScreenToUI(float screenX, float screenY) const {
	auto* r = SleeplessEngine::GetInstance().GetRenderer();
	if (!r) return Vector2f(0.0f, 0.0f);

	int uiW = 0, uiH = 0;
	GetVirtualUISize(uiW, uiH);

	Rectf vp = r->GetViewportRect();
	if (vp.width <= 0.0f || vp.height <= 0.0f) {
		return Vector2f(screenX, screenY);
	}

	const float localX = (screenX - vp.x);
	const float localY = (screenY - vp.y);

	const float u = localX / vp.width;
	const float v = localY / vp.height;

	return Vector2f(u * (float)uiW, v * (float)uiH);
}

UIElement* UISystem::FindTopmostAt(float uiX, float uiY) const {
	UIElement* best = nullptr;
	int bestCanvasOrder = INT32_MIN;
	int bestElementOrder = INT32_MIN;

	for (UIElement* e : m_elements) {
		if (!e) continue;
		if (!e->IsVisible()) continue;
		// If it has a canvas and the canvas is hidden, skip.
		if (auto* cvis = e->GetCanvas()) {
			if (!cvis->IsCanvasVisible()) continue;
		}
		if (!e->ContainsPoint(uiX, uiY)) continue;

		int canvasOrder = 0;
		if (auto* c = e->GetCanvas()) {
			canvasOrder = c->GetSortingOrder();
		}
		const int elementOrder = e->GetOrderInCanvas();

		if (canvasOrder > bestCanvasOrder || (canvasOrder == bestCanvasOrder && elementOrder > bestElementOrder)) {
			best = e;
			bestCanvasOrder = canvasOrder;
			bestElementOrder = elementOrder;
		}
	}
	return best;
}

UISelectable* UISystem::FindFirstSelectable() const {
	UISelectable* best = nullptr;
	int bestCanvasOrder = INT32_MIN;
	int bestOrder = INT32_MIN;
	for (UISelectable* s : m_selectables) {
		if (!s) continue;
		if (!s->IsVisible() || !s->IsInteractable()) continue;
		int canvasOrder = 0;
		if (auto* c = s->GetCanvas()) canvasOrder = c->GetSortingOrder();
		int order = s->GetOrderInCanvas();
		if (canvasOrder > bestCanvasOrder || (canvasOrder == bestCanvasOrder && order > bestOrder)) {
			best = s;
			bestCanvasOrder = canvasOrder;
			bestOrder = order;
		}
	}
	return best;
}

UISelectable* UISystem::FindBestInDirection(UISelectable* from, int dirX, int dirY) const {
	if (!from) return FindFirstSelectable();
	dirX = ClampDir(dirX);
	dirY = ClampDir(dirY);
	if (dirX == 0 && dirY == 0) return from;

	// Explicit neighbours win.
	if (dirY < 0 && from->up) return from->up;
	if (dirY > 0 && from->down) return from->down;
	if (dirX < 0 && from->left) return from->left;
	if (dirX > 0 && from->right) return from->right;

	const Vector2f fromC = from->GetCenter();
	UISelectable* best = nullptr;
	float bestScore = FLT_MAX;

	for (UISelectable* s : m_selectables) {
		if (!s || s == from) continue;
		if (!s->IsVisible() || !s->IsInteractable()) continue;

		const Vector2f c = s->GetCenter();
		const float dx = c.x - fromC.x;
		const float dy = c.y - fromC.y;

		// Must be in the intended half-plane.
		if (dirX != 0) {
			if (dx * (float)dirX <= 0.001f) continue;
		}
		if (dirY != 0) {
			if (dy * (float)dirY <= 0.001f) continue;
		}

		// Score: primary axis distance heavily weighted.
		const float primary = (dirX != 0) ? std::abs(dx) : std::abs(dy);
		const float secondary = (dirX != 0) ? std::abs(dy) : std::abs(dx);
		const float score = primary * 10.0f + secondary;
		if (score < bestScore) {
			bestScore = score;
			best = s;
		}
	}

	return best ? best : from;
}

void UISystem::SetFocused(UISelectable* selectable) {
	if (selectable == m_focused) return;
	if (m_focused) m_focused->OnFocusLost();
	m_focused = selectable;
	if (m_focused) m_focused->OnFocusGained();
}

bool UISystem::ConsumeSubmit() {
	if (Input::IsKeyPressed(Key::Enter) || Input::IsKeyPressed(Key::Space)) return true;
	if (Input::IsGamepadButtonPressed(GamepadButton::South, 0)) return true;
	return false;
}

bool UISystem::ConsumeNavigate(int& outDX, int& outDY) {
	outDX = 0;
	outDY = 0;

	// Keyboard
	if (Input::IsKeyPressed(Key::Up) || Input::IsKeyPressed(Key::W)) outDY = -1;
	else if (Input::IsKeyPressed(Key::Down) || Input::IsKeyPressed(Key::S)) outDY = 1;
	else if (Input::IsKeyPressed(Key::Left) || Input::IsKeyPressed(Key::A)) outDX = -1;
	else if (Input::IsKeyPressed(Key::Right) || Input::IsKeyPressed(Key::D)) outDX = 1;

	// Gamepad DPad (pressed)
	if (outDX == 0 && outDY == 0) {
		if (Input::IsGamepadButtonPressed(GamepadButton::DPadUp, 0)) outDY = -1;
		else if (Input::IsGamepadButtonPressed(GamepadButton::DPadDown, 0)) outDY = 1;
		else if (Input::IsGamepadButtonPressed(GamepadButton::DPadLeft, 0)) outDX = -1;
		else if (Input::IsGamepadButtonPressed(GamepadButton::DPadRight, 0)) outDX = 1;
	}

	// Gamepad stick (with small cooldown so it doesn't spam)
	if (outDX == 0 && outDY == 0) {
		m_stickCooldown -= Time::DeltaTime();
		if (m_stickCooldown <= 0.0f) {
			Vector2f stick = Input::GetGamepadLeftStick(0);
			const float ax = stick.x;
			const float ay = stick.y;
			const float t = 0.60f;
			if (ax <= -t) outDX = -1;
			else if (ax >= t) outDX = 1;
			else if (ay <= -t) outDY = -1;
			else if (ay >= t) outDY = 1;

			if (outDX != 0 || outDY != 0) {
				m_stickCooldown = 0.18f;
			}
		}
	}

	return (outDX != 0 || outDY != 0);
}

void UISystem::Update() {
	// If we had focus but it became invalid (destroyed/hidden/disabled), pick a new one.
	// NOTE: If focus is explicitly null, we allow that (useful for mouse-driven UI).
	if (m_focused && (!m_focused->IsVisible() || !m_focused->IsInteractable())) {
		SetFocused(FindFirstSelectable());
	}

	// Pointer / hover
	Vector2f mouse = Input::GetMousePosition();
	Vector2f uiPos = ScreenToUI(mouse.x, mouse.y);
	UIElement* nowHover = FindTopmostAt(uiPos.x, uiPos.y);

	if (nowHover != m_hovered) {
		UIElement* prev = m_hovered;
		if (prev) prev->OnPointerExit();
		m_hovered = nowHover;
		if (m_hovered) {
			m_hovered->OnPointerEnter();
			if (auto* sel = dynamic_cast<UISelectable*>(m_hovered)) {
				SetFocused(sel);
				m_focusWasSetByHover = true;
			}
		} else {
			// Mouse left the last hovered element: if that element was focused due to hover,
			// clear focus (so buttons don't stay "selected" when the pointer isn't over them).
			if (m_focusWasSetByHover) {
				if (auto* prevSel = dynamic_cast<UISelectable*>(prev)) {
					if (prevSel == m_focused) {
						SetFocused(nullptr);
					}
				}
			}
		}
	}

	// Mouse click
	if (Input::IsMouseButtonPressed(MouseButton::Left)) {
		m_mouseDown = m_hovered;
		if (m_mouseDown) m_mouseDown->OnPointerDown();
	}
	if (Input::IsMouseButtonReleased(MouseButton::Left)) {
		if (m_mouseDown) {
			m_mouseDown->OnPointerUp();
			if (m_mouseDown == m_hovered) {
				m_mouseDown->OnSubmit();
			}
		}
		m_mouseDown = nullptr;
	}

	// Navigation / submit
	int dx = 0, dy = 0;
	if (ConsumeNavigate(dx, dy)) {
		m_focusWasSetByHover = false;
		UISelectable* next = FindBestInDirection(m_focused, dx, dy);
		SetFocused(next);
	}

	if (ConsumeSubmit()) {
		if (m_focused) {
			m_focused->OnSubmit();
		}
		m_focusWasSetByHover = false;
	}
}

void UISystem::Render(Renderer& renderer) {
	// Sort by (canvas order, element order)
	std::vector<UIElement*> sorted;
	sorted.reserve(m_elements.size());
	for (UIElement* e : m_elements) {
		if (!e) continue;
		if (!e->IsVisible()) continue;
		// if it has a canvas and the canvas is hidden, skip
		if (auto* c = e->GetCanvas()) {
			if (!c->IsCanvasVisible()) continue;
		}
		sorted.push_back(e);
	}

	std::sort(sorted.begin(), sorted.end(), [](UIElement* a, UIElement* b) {
		int ao = 0;
		int bo = 0;
		if (a && a->GetCanvas()) ao = a->GetCanvas()->GetSortingOrder();
		if (b && b->GetCanvas()) bo = b->GetCanvas()->GetSortingOrder();
		if (ao != bo) return ao < bo;
		return a->GetOrderInCanvas() < b->GetOrderInCanvas();
	});

	for (UIElement* e : sorted) {
		e->Render(renderer);
	}
}

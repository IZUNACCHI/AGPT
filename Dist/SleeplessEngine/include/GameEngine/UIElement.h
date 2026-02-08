#pragma once

#include "MonoBehaviour.h"
#include "UIPrimitives.h"

class Renderer;
class UICanvas;

// Base component for UI widgets.
//
// Coordinates are in UI space (virtual pixels)
// (0,0) = top-left of virtual screen, +Y goes down.
class UIElement : public MonoBehaviour {
public:
	UIElement();
	~UIElement() override = default;

	void SetRect(const UIRect& r) { m_rect = r; }
	const UIRect& GetRect() const { return m_rect; }

	void SetVisible(bool v) { m_visible = v; }
	bool IsVisible() const { return m_visible && IsActiveAndEnabled(); }

	void SetInteractable(bool v) { m_interactable = v; }
	bool IsInteractable() const { return m_interactable; }

	void SetOrderInCanvas(int order) { m_orderInCanvas = order; }
	int GetOrderInCanvas() const { return m_orderInCanvas; }

	bool ContainsPoint(float uiX, float uiY) const;

	// Cached pointer to nearest parent canvas (may be nullptr).
	UICanvas* GetCanvas() const { return m_canvas; }

	// UI center in UI-space
	Vector2f GetCenter() const;

	// Events
	virtual void OnPointerEnter() {}
	virtual void OnPointerExit() {}
	virtual void OnPointerDown() {}
	virtual void OnPointerUp() {}
	virtual void OnSubmit() {}
	virtual void OnFocusGained() {}
	virtual void OnFocusLost() {}

	// Render hook (called by UISystem)
	virtual void Render(Renderer& renderer) {}

protected:
	void Awake() override;
	void OnDestroy() override;

	//convert UI top-left rect to WORLD top-left for Renderer
	Vector2f UIToWorldTopLeft(const Vector2f& uiTopLeft) const;

private:
	void RefreshCanvasCache();

protected:
	UIRect m_rect{};
	bool m_visible = true;
	bool m_interactable = true;
	int m_orderInCanvas = 0;

	UICanvas* m_canvas = nullptr; // not owned
};

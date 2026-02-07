#pragma once

#include <vector>
#include "Types.hpp"

class Renderer;
class UIElement;
class UISelectable;
class UICanvas;

// Engine-level UI registry + event handling.
//
// - UI elements are regular components (MonoBehaviours) that register/unregister.
// - UISystem is updated from the engine loop so it can consume input every frame.
// - Rendering happens at the end of the frame so UI overlays the world.
class UISystem {
public:
	static UISystem& Get();

	UISystem(const UISystem&) = delete;
	UISystem& operator=(const UISystem&) = delete;

	void RegisterCanvas(UICanvas* canvas);
	void UnregisterCanvas(UICanvas* canvas);

	void RegisterElement(UIElement* element);
	void UnregisterElement(UIElement* element);

	void RegisterSelectable(UISelectable* selectable);
	void UnregisterSelectable(UISelectable* selectable);

	// Called by engine each frame
	void Update();
	void Render(Renderer& renderer);
	// Clears UI registries and state. Call before tearing down input/window.
	void Shutdown();

	// Focus
	UISelectable* GetFocused() const { return m_focused; }
	void SetFocused(UISelectable* selectable);

	// Utility used by UI elements
	static bool GetVirtualUISize(int& outW, int& outH);

private:
	UISystem() = default;

	UISelectable* FindFirstSelectable() const;
	UISelectable* FindBestInDirection(UISelectable* from, int dirX, int dirY) const;

	UIElement* FindTopmostAt(float uiX, float uiY) const;
	Vector2f ScreenToUI(float screenX, float screenY) const;

	bool ConsumeNavigate(int& outDX, int& outDY);
	bool ConsumeSubmit();

private:
	std::vector<UICanvas*> m_canvases;
	std::vector<UIElement*> m_elements;
	std::vector<UISelectable*> m_selectables;

	UISelectable* m_focused = nullptr;
	UIElement* m_hovered = nullptr;
	UIElement* m_mouseDown = nullptr;
	bool m_focusWasSetByHover = false;

	// Gamepad stick nav repeat guard
	float m_stickCooldown = 0.0f;
};

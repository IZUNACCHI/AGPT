#pragma once

#include "UIElement.h"

class UISelectable : public UIElement {
public:
	UISelectable();
	~UISelectable() override = default;

	// Optional explicit neighbours for navigation.
	UISelectable* up = nullptr;
	UISelectable* down = nullptr;
	UISelectable* left = nullptr;
	UISelectable* right = nullptr;

	bool IsFocused() const { return m_focused; }

	void OnFocusGained() override { m_focused = true; }
	void OnFocusLost() override { m_focused = false; }

protected:
	void Awake() override;
	void OnDestroy() override;

private:
	bool m_focused = false;
};

#pragma once

#include "Component.h"

class RenderSystem;

// Base class for components that render something
class RenderableComponent : public Component {
public:
	explicit RenderableComponent(const std::string& name = "RenderableComponent")
		: Component(name) {
	}

	void SetVisible(bool visible) { m_isVisible = visible; }
	bool IsVisible() const { return m_isVisible; }

protected:
	void DestroyImmediateInternal() override;

private:
	bool m_isVisible = true; // Whether the component is visible
};

#pragma once

#include "Component.h"
#include "Renderer.h"
#include "Types.hpp"


// Base class for all renderable components
class RenderableComponent : public Component {
public:
	virtual ~RenderableComponent() = default;

	// Override this to implement rendering
	virtual void Draw() override = 0;

	// Layer and order for sorting
	void SetRenderOrder(int order) { m_renderOrder = order; }
	int GetRenderOrder() const { return m_renderOrder; }

	// Visibility
	void SetVisible(bool visible) { m_visible = visible; }
	bool IsVisible() const { return m_visible && IsEnabled(); }

protected:
	int m_renderOrder = 0;
	bool m_visible = true;

	// Helper to get renderer
	static Renderer* GetRenderer();
};

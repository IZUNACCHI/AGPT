#pragma once

#include "MonoBehaviour.h"

// Marks a UI root.
//
// A canvas is just a logical grouping + a sorting order (drawn later = on top).
class UICanvas : public MonoBehaviour {
public:
	UICanvas();
	~UICanvas() override = default;

	void SetSortingOrder(int order) { m_sortingOrder = order; }
	int GetSortingOrder() const { return m_sortingOrder; }

	void SetCanvasVisible(bool v) { m_visible = v; }
	bool IsCanvasVisible() const { return m_visible; }

protected:
	void Awake() override;
	void OnDestroy() override;

private:
	int m_sortingOrder = 0;
	bool m_visible = true;
};

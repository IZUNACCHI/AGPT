#pragma once

#include "Types.hpp"

// UI rectangle in VIRTUAL PIXELS (top-left origin, +Y down)
struct UIRect {
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
};

inline Vector2f UIGetCenter(const UIRect& r) {
	return Vector2f(r.x + r.w * 0.5f, r.y + r.h * 0.5f);
}

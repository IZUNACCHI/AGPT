#pragma once
#include <cstdint>
#include <algorithm>
#include "Types.hpp" 

namespace Viewport {

	// Bitmask for specifying sides of the viewport
	enum class Side : uint8_t {
		None = 0,
		Left = 1 << 0,
		Right = 1 << 1,
		Top = 1 << 2,
		Bottom = 1 << 3,
		All = Left | Right | Top | Bottom
	};

	inline Side operator|(Side a, Side b) {
		return static_cast<Side>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
	}
	inline Side operator&(Side a, Side b) {
		return static_cast<Side>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
	}
	inline Side& operator|=(Side& a, Side b) { a = a | b; return a; }
	inline bool Any(Side s) { return static_cast<uint8_t>(s) != 0; }

	// WORLD bounds of the visible game area (virtual resolution).
	// left=-w/2, right=+w/2, bottom=-h/2, top=+h/2 (Y is up)
	struct Bounds {
		float left, right, bottom, top;
	};

	inline Bounds GetWorldBounds(const Vector2i& gameSize) {
		const float hw = gameSize.x * 0.5f;
		const float hh = gameSize.y * 0.5f;
		return Bounds{ -hw, +hw, -hh, +hh };
	}

	
	inline Side OutsideSidesPoint(const Vector2f& p, const Vector2i& gameSize) {
		const Bounds b = GetWorldBounds(gameSize);
		Side out = Side::None;

		if (p.x < b.left)  out |= Side::Left;
		if (p.x > b.right) out |= Side::Right;
		if (p.y > b.top)   out |= Side::Top;
		if (p.y < b.bottom)out |= Side::Bottom;

		return out;
	}

	inline bool IsPointOutside(const Vector2f& p, const Vector2i& gameSize, Side sides = Side::All) {
		return Any(OutsideSidesPoint(p, gameSize) & sides);
	}

	inline Vector2f ClampPoint(const Vector2f& p, const Vector2i& gameSize, Side sides = Side::All) {
		const Bounds b = GetWorldBounds(gameSize);
		Vector2f r = p;

		if (Any(sides & Side::Left))   r.x = std::max(r.x, b.left);
		if (Any(sides & Side::Right))  r.x = std::min(r.x, b.right);
		if (Any(sides & Side::Top))    r.y = std::min(r.y, b.top);
		if (Any(sides & Side::Bottom)) r.y = std::max(r.y, b.bottom);

		return r;
	}

	struct WorldRect {
		Vector2f topLeft;
		Vector2f size; // size.x >= 0, size.y >= 0
	};

	inline float RectLeft(const WorldRect& r) { return r.topLeft.x; }
	inline float RectRight(const WorldRect& r) { return r.topLeft.x + r.size.x; }
	inline float RectTop(const WorldRect& r) { return r.topLeft.y; }
	inline float RectBottom(const WorldRect& r) { return r.topLeft.y - r.size.y; }

	inline Side OutsideSidesRect(const WorldRect& r, const Vector2i& gameSize) {
		const Bounds b = GetWorldBounds(gameSize);
		Side out = Side::None;

		if (RectRight(r) < b.left)   out |= Side::Left;    // fully left
		if (RectLeft(r) > b.right)  out |= Side::Right;   // fully right
		if (RectBottom(r) > b.top)    out |= Side::Top;     // fully above
		if (RectTop(r) < b.bottom) out |= Side::Bottom;  // fully below

		return out;
	}

	//Overhang = partially outside
	inline Side OverhangSidesRect(const WorldRect& r, const Vector2i& gameSize) {
		const Bounds b = GetWorldBounds(gameSize);
		Side out = Side::None;

		if (RectLeft(r) < b.left)   out |= Side::Left;
		if (RectRight(r) > b.right)  out |= Side::Right;
		if (RectTop(r) > b.top)    out |= Side::Top;
		if (RectBottom(r) < b.bottom) out |= Side::Bottom;

		return out;
	}

	inline bool IsRectOutside(const WorldRect& r, const Vector2i& gameSize, Side sides = Side::All) {
		return Any(OutsideSidesRect(r, gameSize) & sides);
	}

	inline bool IsRectOverhanging(const WorldRect& r, const Vector2i& gameSize, Side sides = Side::All) {
		return Any(OverhangSidesRect(r, gameSize) & sides);
	}

	// Clamp a rect so it stays fully inside
	inline WorldRect ClampRect(WorldRect r, const Vector2i& gameSize, Side sides = Side::All) {
		const Bounds b = GetWorldBounds(gameSize);

		// Horizontal
		if (Any(sides & Side::Left)) {
			const float dx = b.left - RectLeft(r);
			if (dx > 0.0f) r.topLeft.x += dx;
		}
		if (Any(sides & Side::Right)) {
			const float dx = RectRight(r) - b.right;
			if (dx > 0.0f) r.topLeft.x -= dx;
		}

		// Vertical (+Y up)
		if (Any(sides & Side::Top)) {
			const float dy = RectTop(r) - b.top;   // too high
			if (dy > 0.0f) r.topLeft.y -= dy;
		}
		if (Any(sides & Side::Bottom)) {
			const float dy = b.bottom - RectBottom(r); // too low
			if (dy > 0.0f) r.topLeft.y += dy;
		}

		return r;
	}

} 

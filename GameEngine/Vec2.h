#pragma once
#include <cmath>
#include <ostream>

class Vec2
{
public:
	float x;
	float y;

	// --- Constructors ---
	constexpr Vec2() : x(0), y(0) {}
	constexpr Vec2(float x, float y) : x(x), y(y) {}

	// Zero / One / Directions
	static constexpr Vec2 Zero() { return Vec2(0.f, 0.f); }
	static constexpr Vec2 One() { return Vec2(1.f, 1.f); }
	static constexpr Vec2 Up() { return Vec2(0.f, -1.f); }
	static constexpr Vec2 Down() { return Vec2(0.f, 1.f); }
	static constexpr Vec2 Left() { return Vec2(-1.f, 0.f); }
	static constexpr Vec2 Right() { return Vec2(1.f, 0.f); }

	// --- Operator Overloads ---
	constexpr Vec2 operator+(const Vec2& v) const { return { x + v.x, y + v.y }; }
	constexpr Vec2 operator-(const Vec2& v) const { return { x - v.x, y - v.y }; }
	constexpr Vec2 operator*(float s) const { return { x * s, y * s }; }
	constexpr Vec2 operator/(float s) const { return { x / s, y / s }; }

	constexpr Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
	constexpr Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
	constexpr Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
	constexpr Vec2& operator/=(float s) { x /= s; y /= s; return *this; }

	constexpr bool operator==(const Vec2& v) const { return x == v.x && y == v.y; }
	constexpr bool operator!=(const Vec2& v) const { return !(*this == v); }

	// Unary minus
	constexpr Vec2 operator-() const { return Vec2(-x, -y); }

	// --- Magnitude / Normalization ---
	float Length() const {
		return std::sqrt(x * x + y * y);
	}

	float LengthSquared() const {
		return x * x + y * y;
	}

	Vec2 Normalized() const {
		float len = Length();
		if (len == 0.f) return Vec2(0.f, 0.f);
		return Vec2(x / len, y / len);
	}

	Vec2& Normalize() {
		float len = Length();
		if (len != 0.f) { x /= len; y /= len; }
		return *this;
	}

	// --- Dot / Cross ---
	float Dot(const Vec2& v) const {
		return x * v.x + y * v.y;
	}

	// 2D cross product result (scalar)
	float Cross(const Vec2& v) const {
		return x * v.y - y * v.x;
	}

	// --- Distance ---
	static float Distance(const Vec2& a, const Vec2& b) {
		return (a - b).Length();
	}

	static float DistanceSquared(const Vec2& a, const Vec2& b) {
		return (a - b).LengthSquared();
	}

	// --- Clamp magnitude ---
	Vec2 Clamped(float maxLen) const {
		float len = Length();
		if (len > maxLen)
			return Normalized() * maxLen;
		return *this;
	}

	// --- Lerp ---
	static Vec2 Lerp(const Vec2& a, const Vec2& b, float t) {
		return a + (b - a) * t;
	}

	// --- Angle ---
	float Angle() const { return std::atan2(y, x); }

	// --- Rotate (radians) ---
	Vec2 Rotated(float radians) const {
		float c = std::cos(radians);
		float s = std::sin(radians);
		return Vec2(
			x * c - y * s,
			x * s + y * c
		);
	}

	// --- Stream output ---
	friend std::ostream& operator<<(std::ostream& os, const Vec2& v) {
		os << "Vec2(" << v.x << ", " << v.y << ")";
		return os;
	}
};

// scalar * Vec2
inline Vec2 operator*(float s, const Vec2& v) { return v * s; }


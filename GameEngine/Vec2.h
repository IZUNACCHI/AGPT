#pragma once
#include <cmath>

struct Vec2 {
    float x = 0.f;
    float y = 0.f;
    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
};

inline float Dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
inline float Length(const Vec2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }
inline Vec2 Normalize(const Vec2& v) {
    float len = Length(v);
    return (len > 1e-6f) ? Vec2(v.x / len, v.y / len) : Vec2(0.f, 0.f);
}
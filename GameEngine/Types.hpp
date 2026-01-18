#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <type_traits>
#include <array>
#include <cstdint>

namespace Engine {
	namespace Math {

		// Constants
		template<typename T>
		struct Constants {
			static constexpr T Pi = static_cast<T>(3.14159265358979323846);
			static constexpr T TwoPi = static_cast<T>(2) * Pi;
			static constexpr T HalfPi = Pi / static_cast<T>(2);
			static constexpr T Epsilon = std::numeric_limits<T>::epsilon();
			static constexpr T Deg2Rad = Pi / static_cast<T>(180);
			static constexpr T Rad2Deg = static_cast<T>(180) / Pi;
		};

		// Utility functions
		template<typename T>
		inline T Clamp(T value, T min, T max) {
			return value < min ? min : (value > max ? max : value);
		}

		template<typename T>
		inline T Lerp(T a, T b, T t) {
			return a + (b - a) * t;
		}

		template<typename T>
		inline T SmoothStep(T edge0, T edge1, T x) {
			x = Clamp((x - edge0) / (edge1 - edge0), static_cast<T>(0), static_cast<T>(1));
			return x * x * (static_cast<T>(3) - static_cast<T>(2) * x);
		}

		template<typename T>
		inline T Max(T a, T b) {
			return a > b ? a : b;
		}

		template<typename T>
		inline T Min(T a, T b) {
			return a < b ? a : b;
		}

		template<typename T>
		inline T Abs(T value) {
			return value < static_cast<T>(0) ? -value : value;
		}

		template<typename T>
		inline bool Approximately(T a, T b, T epsilon = Constants<T>::Epsilon) {
			return Abs(a - b) <= epsilon;
		}

	} // namespace Math

	// Vector declarations first (forward declare if needed for Matrix)
	template<typename T> class Vector2;
	template<typename T> class Vector3;
	template<typename T> class Vector4;

	// Common typedefs (forward declarations)
	using Vector2f = Vector2<float>;
	using Vector2i = Vector2<int>;
	using Vector2u = Vector2<unsigned int>;
	using Vector2d = Vector2<double>;

	// Vector2 definition
	template<typename T>
	class Vector2 {
	public:
		T x, y;

		// Constructors
		Vector2() : x(0), y(0) {}
		Vector2(T x, T y) : x(x), y(y) {}
		explicit Vector2(T scalar) : x(scalar), y(scalar) {}

		// Copy constructor
		Vector2(const Vector2& other) = default;

		// Assignment operator
		Vector2& operator=(const Vector2& other) = default;

		// Access operators
		T& operator[](size_t index) {
			return index == 0 ? x : y;
		}

		const T& operator[](size_t index) const {
			return index == 0 ? x : y;
		}

		// Arithmetic operators
		Vector2 operator+(const Vector2& other) const {
			return Vector2(x + other.x, y + other.y);
		}

		Vector2 operator-(const Vector2& other) const {
			return Vector2(x - other.x, y - other.y);
		}

		Vector2 operator*(T scalar) const {
			return Vector2(x * scalar, y * scalar);
		}

		Vector2 operator/(T scalar) const {
			return Vector2(x / scalar, y / scalar);
		}

		Vector2& operator+=(const Vector2& other) {
			x += other.x;
			y += other.y;
			return *this;
		}

		Vector2& operator-=(const Vector2& other) {
			x -= other.x;
			y -= other.y;
			return *this;
		}

		Vector2& operator*=(T scalar) {
			x *= scalar;
			y *= scalar;
			return *this;
		}

		Vector2& operator/=(T scalar) {
			x /= scalar;
			y /= scalar;
			return *this;
		}

		Vector2 operator-() const {
			return Vector2(-x, -y);
		}

		// Comparison operators
		bool operator==(const Vector2& other) const {
			return Math::Approximately(x, other.x) && Math::Approximately(y, other.y);
		}

		bool operator!=(const Vector2& other) const {
			return !(*this == other);
		}

		// Vector operations
		T Dot(const Vector2& other) const {
			return x * other.x + y * other.y;
		}

		T Cross(const Vector2& other) const {
			return x * other.y - y * other.x;
		}

		T LengthSquared() const {
			return x * x + y * y;
		}

		T Length() const {
			return static_cast<T>(std::sqrt(LengthSquared()));
		}

		Vector2 Normalized() const {
			T len = Length();
			if (len > Math::Constants<T>::Epsilon) {
				return *this / len;
			}
			return Vector2();
		}

		void Normalize() {
			T len = Length();
			if (len > Math::Constants<T>::Epsilon) {
				x /= len;
				y /= len;
			}
		}

		T DistanceSquared(const Vector2& other) const {
			T dx = x - other.x;
			T dy = y - other.y;
			return dx * dx + dy * dy;
		}

		T Distance(const Vector2& other) const {
			return static_cast<T>(std::sqrt(DistanceSquared(other)));
		}

		// Static methods
		static Vector2 Zero() { return Vector2(0, 0); }
		static Vector2 One() { return Vector2(1, 1); }
		static Vector2 Up() { return Vector2(0, 1); }
		static Vector2 Down() { return Vector2(0, -1); }
		static Vector2 Left() { return Vector2(-1, 0); }
		static Vector2 Right() { return Vector2(1, 0); }

		// Cast operators
		template<typename U>
		explicit operator Vector2<U>() const {
			return Vector2<U>(static_cast<U>(x), static_cast<U>(y));
		}
	};

	// Scalar multiplication (scalar * vector)
	template<typename T>
	Vector2<T> operator*(T scalar, const Vector2<T>& vector) {
		return vector * scalar;
	}

	// Vector3 definition
	template<typename T>
	class Vector3 {
	public:
		T x, y, z;

		// Constructors
		Vector3() : x(0), y(0), z(0) {}
		Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
		Vector3(const Vector2<T>& v2, T z = 0) : x(v2.x), y(v2.y), z(z) {}
		explicit Vector3(T scalar) : x(scalar), y(scalar), z(scalar) {}

		// Access operators
		T& operator[](size_t index) {
			return index == 0 ? x : (index == 1 ? y : z);
		}

		const T& operator[](size_t index) const {
			return index == 0 ? x : (index == 1 ? y : z);
		}

		// Arithmetic operators (similar to Vector2 but with z component)
		Vector3 operator+(const Vector3& other) const {
			return Vector3(x + other.x, y + other.y, z + other.z);
		}

		Vector3 operator-(const Vector3& other) const {
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		Vector3 operator*(T scalar) const {
			return Vector3(x * scalar, y * scalar, z * scalar);
		}

		Vector3 operator/(T scalar) const {
			return Vector3(x / scalar, y / scalar, z / scalar);
		}

		Vector3& operator+=(const Vector3& other) {
			x += other.x; y += other.y; z += other.z;
			return *this;
		}

		Vector3& operator-=(const Vector3& other) {
			x -= other.x; y -= other.y; z -= other.z;
			return *this;
		}

		Vector3& operator*=(T scalar) {
			x *= scalar; y *= scalar; z *= scalar;
			return *this;
		}

		Vector3& operator/=(T scalar) {
			x /= scalar; y /= scalar; z /= scalar;
			return *this;
		}

		Vector3 operator-() const {
			return Vector3(-x, -y, -z);
		}

		// Comparison operators
		bool operator==(const Vector3& other) const {
			return Math::Approximately(x, other.x) &&
				Math::Approximately(y, other.y) &&
				Math::Approximately(z, other.z);
		}

		bool operator!=(const Vector3& other) const {
			return !(*this == other);
		}

		// Vector operations
		T Dot(const Vector3& other) const {
			return x * other.x + y * other.y + z * other.z;
		}

		Vector3 Cross(const Vector3& other) const {
			return Vector3(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			);
		}

		T LengthSquared() const {
			return x * x + y * y + z * z;
		}

		T Length() const {
			return static_cast<T>(std::sqrt(LengthSquared()));
		}

		Vector3 Normalized() const {
			T len = Length();
			if (len > Math::Constants<T>::Epsilon) {
				return *this / len;
			}
			return Vector3();
		}

		void Normalize() {
			T len = Length();
			if (len > Math::Constants<T>::Epsilon) {
				x /= len; y /= len; z /= len;
			}
		}

		T DistanceSquared(const Vector3& other) const {
			T dx = x - other.x;
			T dy = y - other.y;
			T dz = z - other.z;
			return dx * dx + dy * dy + dz * dz;
		}

		T Distance(const Vector3& other) const {
			return static_cast<T>(std::sqrt(DistanceSquared(other)));
		}

		// Static methods
		static Vector3 Zero() { return Vector3(0, 0, 0); }
		static Vector3 One() { return Vector3(1, 1, 1); }
		static Vector3 Up() { return Vector3(0, 1, 0); }
		static Vector3 Down() { return Vector3(0, -1, 0); }
		static Vector3 Left() { return Vector3(-1, 0, 0); }
		static Vector3 Right() { return Vector3(1, 0, 0); }
		static Vector3 Forward() { return Vector3(0, 0, 1); }
		static Vector3 Back() { return Vector3(0, 0, -1); }

		// Conversion to Vector2 (drops z component)
		Vector2<T> ToVector2() const {
			return Vector2<T>(x, y);
		}

		// Cast operators
		template<typename U>
		explicit operator Vector3<U>() const {
			return Vector3<U>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z));
		}
	};

	// Common typedefs for Vector3
	using Vector3f = Vector3<float>;
	using Vector3i = Vector3<int>;
	using Vector3u = Vector3<unsigned int>;
	using Vector3d = Vector3<double>;

	// Scalar multiplication for Vector3
	template<typename T>
	Vector3<T> operator*(T scalar, const Vector3<T>& vector) {
		return vector * scalar;
	}

	// Vector4 definition
	template<typename T>
	class Vector4 {
	public:
		T x, y, z, w;

		// Constructors
		Vector4() : x(0), y(0), z(0), w(0) {}
		Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
		Vector4(const Vector3<T>& v3, T w = 0) : x(v3.x), y(v3.y), z(v3.z), w(w) {}
		explicit Vector4(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

		// Access operators
		T& operator[](size_t index) {
			return index == 0 ? x : (index == 1 ? y : (index == 2 ? z : w));
		}

		const T& operator[](size_t index) const {
			return index == 0 ? x : (index == 1 ? y : (index == 2 ? z : w));
		}

		// Arithmetic operators
		Vector4 operator+(const Vector4& other) const {
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
		}

		Vector4 operator-(const Vector4& other) const {
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
		}

		Vector4 operator*(T scalar) const {
			return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
		}

		Vector4 operator/(T scalar) const {
			return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
		}

		Vector4& operator+=(const Vector4& other) {
			x += other.x; y += other.y; z += other.z; w += other.w;
			return *this;
		}

		Vector4& operator-=(const Vector4& other) {
			x -= other.x; y -= other.y; z -= other.z; w -= other.w;
			return *this;
		}

		Vector4& operator*=(T scalar) {
			x *= scalar; y *= scalar; z *= scalar; w *= scalar;
			return *this;
		}

		Vector4& operator/=(T scalar) {
			x /= scalar; y /= scalar; z /= scalar; w /= scalar;
			return *this;
		}

		Vector4 operator-() const {
			return Vector4(-x, -y, -z, -w);
		}

		// Comparison operators
		bool operator==(const Vector4& other) const {
			return Math::Approximately(x, other.x) &&
				Math::Approximately(y, other.y) &&
				Math::Approximately(z, other.z) &&
				Math::Approximately(w, other.w);
		}

		bool operator!=(const Vector4& other) const {
			return !(*this == other);
		}

		// Vector operations
		T Dot(const Vector4& other) const {
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		T LengthSquared() const {
			return x * x + y * y + z * z + w * w;
		}

		T Length() const {
			return static_cast<T>(std::sqrt(LengthSquared()));
		}

		Vector4 Normalized() const {
			T len = Length();
			if (len > Math::Constants<T>::Epsilon) {
				return *this / len;
			}
			return Vector4();
		}

		void Normalize() {
			T len = Length();
			if (len > Math::Constants<T>::Epsilon) {
				x /= len; y /= len; z /= len; w /= len;
			}
		}

		// Static methods
		static Vector4 Zero() { return Vector4(0, 0, 0, 0); }
		static Vector4 One() { return Vector4(1, 1, 1, 1); }

		// Conversion to Vector3 (drops w component)
		Vector3<T> ToVector3() const {
			return Vector3<T>(x, y, z);
		}

		// Cast operators
		template<typename U>
		explicit operator Vector4<U>() const {
			return Vector4<U>(static_cast<U>(x), static_cast<U>(y),
				static_cast<U>(z), static_cast<U>(w));
		}
	};

	// Common typedefs for Vector4
	using Vector4f = Vector4<float>;
	using Vector4i = Vector4<int>;
	using Vector4u = Vector4<unsigned int>;
	using Vector4d = Vector4<double>;

	// Scalar multiplication for Vector4
	template<typename T>
	Vector4<T> operator*(T scalar, const Vector4<T>& vector) {
		return vector * scalar;
	}

	// Color class (uses Vector4)
	class Color {
	public:
		uint8_t r, g, b, a;

		// Constructors
		Color() : r(0), g(0), b(0), a(255) {}
		Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
			: r(red), g(green), b(blue), a(alpha) {
		}

		explicit Color(uint32_t hex) {
			r = static_cast<uint8_t>((hex >> 24) & 0xFF);
			g = static_cast<uint8_t>((hex >> 16) & 0xFF);
			b = static_cast<uint8_t>((hex >> 8) & 0xFF);
			a = static_cast<uint8_t>(hex & 0xFF);
		}

		// Named color constructors
		static Color Black(uint8_t alpha = 255) { return Color(0, 0, 0, alpha); }
		static Color White(uint8_t alpha = 255) { return Color(255, 255, 255, alpha); }
		static Color Red(uint8_t alpha = 255) { return Color(255, 0, 0, alpha); }
		static Color Green(uint8_t alpha = 255) { return Color(0, 255, 0, alpha); }
		static Color Blue(uint8_t alpha = 255) { return Color(0, 0, 255, alpha); }
		static Color Yellow(uint8_t alpha = 255) { return Color(255, 255, 0, alpha); }
		static Color Magenta(uint8_t alpha = 255) { return Color(255, 0, 255, alpha); }
		static Color Cyan(uint8_t alpha = 255) { return Color(0, 255, 255, alpha); }
		static Color Transparent() { return Color(0, 0, 0, 0); }

		// Conversion methods
		Vector4f ToVector4f() const {
			return Vector4f(
				static_cast<float>(r) / 255.0f,
				static_cast<float>(g) / 255.0f,
				static_cast<float>(b) / 255.0f,
				static_cast<float>(a) / 255.0f
			);
		}

		Vector4i ToVector4i() const {
			return Vector4i(r, g, b, a);
		}

		// Color operations
		Color Lerp(const Color& other, float t) const {
			t = Math::Clamp(t, 0.0f, 1.0f);
			return Color(
				static_cast<uint8_t>(Math::Lerp(static_cast<float>(r), static_cast<float>(other.r), t)),
				static_cast<uint8_t>(Math::Lerp(static_cast<float>(g), static_cast<float>(other.g), t)),
				static_cast<uint8_t>(Math::Lerp(static_cast<float>(b), static_cast<float>(other.b), t)),
				static_cast<uint8_t>(Math::Lerp(static_cast<float>(a), static_cast<float>(other.a), t))
			);
		}

		// Create color with modified alpha
		Color WithAlpha(uint8_t alpha) const {
			return Color(r, g, b, alpha);
		}

		// Comparison operators
		bool operator==(const Color& other) const {
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}

		bool operator!=(const Color& other) const {
			return !(*this == other);
		}

		// Color keying support
		bool IsTransparent() const {
			return a == 0;
		}

		bool IsOpaque() const {
			return a == 255;
		}

		// Convert to hex (ARGB format)
		uint32_t ToHex() const {
			return (static_cast<uint32_t>(r) << 24) |
				(static_cast<uint32_t>(g) << 16) |
				(static_cast<uint32_t>(b) << 8) |
				static_cast<uint32_t>(a);
		}
	};

	// Rect definition
	template<typename T>
	class Rect {
	public:
		T x, y, width, height;

		// Constructors
		Rect() : x(0), y(0), width(0), height(0) {}
		Rect(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}
		Rect(const Vector2<T>& position, const Vector2<T>& windowSize)
			: x(position.x), y(position.y), width(windowSize.x), height(windowSize.y) {
		}

		// Getters
		T GetLeft() const { return x; }
		T GetRight() const { return x + width; }
		T GetTop() const { return y; }
		T GetBottom() const { return y + height; }
		Vector2<T> GetPosition() const { return Vector2<T>(x, y); }
		Vector2<T> GetSize() const { return Vector2<T>(width, height); }
		Vector2<T> GetCenter() const { return Vector2<T>(x + width / 2, y + height / 2); }

		// Setters
		void SetPosition(T newX, T newY) { x = newX; y = newY; }
		void SetPosition(const Vector2<T>& position) { x = position.x; y = position.y; }
		void SetSize(T newWidth, T newHeight) { width = newWidth; height = newHeight; }
		void SetSize(const Vector2<T>& windowSize) { width = windowSize.x; height = windowSize.y; }

		// Checks
		bool Contains(T pointX, T pointY) const {
			return pointX >= x && pointX <= x + width &&
				pointY >= y && pointY <= y + height;
		}

		bool Contains(const Vector2<T>& point) const {
			return Contains(point.x, point.y);
		}

		bool Intersects(const Rect& other) const {
			return !(other.x > x + width ||
				other.x + other.width < x ||
				other.y > y + height ||
				other.y + other.height < y);
		}

		// Transformations
		Rect Intersection(const Rect& other) const {
			T left = Math::Max(x, other.x);
			T top = Math::Max(y, other.y);
			T right = Math::Min(x + width, other.x + other.width);
			T bottom = Math::Min(y + height, other.y + other.height);

			if (right > left && bottom > top) {
				return Rect(left, top, right - left, bottom - top);
			}
			return Rect(); // Empty rect
		}

		// Operators
		bool operator==(const Rect& other) const {
			return Math::Approximately(x, other.x) &&
				Math::Approximately(y, other.y) &&
				Math::Approximately(width, other.width) &&
				Math::Approximately(height, other.height);
		}

		bool operator!=(const Rect& other) const {
			return !(*this == other);
		}

		Rect operator+(const Vector2<T>& offset) const {
			return Rect(x + offset.x, y + offset.y, width, height);
		}

		Rect operator-(const Vector2<T>& offset) const {
			return Rect(x - offset.x, y - offset.y, width, height);
		}

		Rect& operator+=(const Vector2<T>& offset) {
			x += offset.x;
			y += offset.y;
			return *this;
		}

		Rect& operator-=(const Vector2<T>& offset) {
			x -= offset.x;
			y -= offset.y;
			return *this;
		}

		// Cast operators
		template<typename U>
		explicit operator Rect<U>() const {
			return Rect<U>(static_cast<U>(x), static_cast<U>(y),
				static_cast<U>(width), static_cast<U>(height));
		}

		// Static methods
		static Rect Empty() { return Rect(); }
		static Rect FromCorners(T left, T top, T right, T bottom) {
			return Rect(left, top, right - left, bottom - top);
		}
	};

	// Common typedefs for Rect
	using Recti = Rect<int>;
	using Rectf = Rect<float>;
	using Rectu = Rect<unsigned int>;
	using Rectd = Rect<double>;

	// Matrix3x3f definition - MUST COME AFTER Vector2f is fully defined
	class Matrix3x3f {
	public:
		float m[9]; // Column-major: m[0], m[3], m[6] = col1, etc.

		Matrix3x3f() {
			for (int i = 0; i < 9; ++i) m[i] = 0.0f;
		}

		Matrix3x3f(const Matrix3x3f& other) = default;
		Matrix3x3f& operator=(const Matrix3x3f& other) = default;

		static Matrix3x3f Identity() {
			Matrix3x3f mat;
			mat.m[0] = 1.0f; mat.m[1] = 0.0f; mat.m[2] = 0.0f;
			mat.m[3] = 0.0f; mat.m[4] = 1.0f; mat.m[5] = 0.0f;
			mat.m[6] = 0.0f; mat.m[7] = 0.0f; mat.m[8] = 1.0f;
			return mat;
		}

		static Matrix3x3f Translation(const Vector2f& translation) {
			Matrix3x3f mat = Identity();
			mat.m[6] = translation.x;
			mat.m[7] = translation.y;
			return mat;
		}

		static Matrix3x3f Rotation(float angle) {
			float rad = angle * Math::Constants<float>::Deg2Rad;
			float c = std::cos(rad);
			float s = std::sin(rad);

			Matrix3x3f mat = Identity();
			mat.m[0] = c; mat.m[1] = s;
			mat.m[3] = -s; mat.m[4] = c;
			return mat;
		}

		static Matrix3x3f Scale(const Vector2f& scale) {
			Matrix3x3f mat = Identity();
			mat.m[0] = scale.x;
			mat.m[4] = scale.y;
			return mat;
		}

		Matrix3x3f operator*(const Matrix3x3f& other) const {
			Matrix3x3f result;

			// 3x3 matrix multiplication
			for (int row = 0; row < 3; ++row) {
				for (int col = 0; col < 3; ++col) {
					result.m[row * 3 + col] = 0.0f;
					for (int k = 0; k < 3; ++k) {
						result.m[row * 3 + col] +=
							m[row * 3 + k] * other.m[k * 3 + col];
					}
				}
			}

			return result;
		}

		Vector2f operator*(const Vector2f& vec) const {
			float x = m[0] * vec.x + m[3] * vec.y + m[6];
			float y = m[1] * vec.x + m[4] * vec.y + m[7];
			float w = m[2] * vec.x + m[5] * vec.y + m[8];

			if (w != 0.0f && w != 1.0f) {
				x /= w;
				y /= w;
			}

			return Vector2f(x, y);
		}
	};

} // namespace Engine

// Global aliases (outside Engine namespace)
using Vector2f = Engine::Vector2f;
using Vector2i = Engine::Vector2i;
using Vector2u = Engine::Vector2u;
using Vector2d = Engine::Vector2d;

using Vector3f = Engine::Vector3f;
using Vector3i = Engine::Vector3i;
using Vector3u = Engine::Vector3u;
using Vector3d = Engine::Vector3d;

using Vector4f = Engine::Vector4f;
using Vector4i = Engine::Vector4i;
using Vector4u = Engine::Vector4u;
using Vector4d = Engine::Vector4d;

using Rectf = Engine::Rectf;
using Recti = Engine::Recti;
using Rectu = Engine::Rectu;
using Rectd = Engine::Rectd;

using Color = Engine::Color;
using Matrix3x3f = Engine::Matrix3x3f;
#pragma once

#include "Types.hpp"

class BitmapFont;

struct UIButtonStyle {
	Vector4i bgNormal{ 40, 40, 40, 255 };
	Vector4i bgHover{ 60, 60, 60, 255 };
	Vector4i bgPressed{ 25, 25, 25, 255 };
	Vector4i bgDisabled{ 20, 20, 20, 160 };
	Vector4i bgFocused{ 70, 70, 70, 255 };

	Vector4i textColor{ 230, 230, 230, 255 };
	Vector4i borderColor{ 255, 255, 255, 255 };

	float borderThickness = 2.0f;
	float padding = 8.0f;

	// Not owned
	BitmapFont* font = nullptr;
};

struct UIProgressBarStyle {
	Vector4i backColor{ 30, 30, 30, 255 };
	Vector4i fillColor{ 0, 200, 90, 255 };
	Vector4i borderColor{ 255, 255, 255, 255 };
	float borderThickness = 2.0f;
	bool showBorder = true;
	bool leftToRight = true;
};

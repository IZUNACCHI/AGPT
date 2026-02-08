#pragma once
#include <memory>
#include <vector>

#include "Types.hpp" 

// key enum for keyboard keys
enum class Key {
	Unknown = 0,
	A, B, C, D, E, F, G,
	H, I, J, K, L, M, N,
	O, P, Q, R, S, T, U,
	V, W, X, Y, Z,
	Num0, Num1, Num2, Num3, Num4,
	Num5, Num6, Num7, Num8, Num9,
	Space, Enter, Escape,
	LeftShift, RightShift,
	LeftCtrl, RightCtrl,
	LeftAlt, RightAlt,
	Up, Down, Left, Right,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	Backspace, Tab, Capslock,
	Minus, Equals, LeftBracket, RightBracket, Backslash,
	Semicolon, Apostrophe, Grave, Comma, Period, Slash,
	Numlock, Scrolllock, Printscreen,
	Insert, Delete, Home, End, PageUp, PageDown,
	KP0, KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9,
	KPPeriod, KPDivide, KPMultiply, KPMinus, KPPlus, KPEnter,
	VolumeUp, VolumeDown, Mute,
	Max
};

// mouse button enum
enum class MouseButton {
	Invalid = -1,
	Left = 0,
	Right,
	Middle,
	X1,
	X2,
	Max
};

enum class KeyState { Up, Down, Pressed, Released };
enum class ButtonState { Up, Down, Pressed, Released };

enum class GamepadAxis : int {
	Invalid = -1,
	LeftX = 0, LeftY = 1,
	RightX = 2, RightY = 3,
	LeftTrigger = 4, RightTrigger = 5,
	Max
};

enum class GamepadButton : int {
	Invalid = -1,
	South = 0,      // A / Cross
	East = 1,       // B / Circle
	West = 2,       // X / Square
	North = 3,      // Y / Triangle
	Back = 4,
	Guide = 5,
	Start = 6,
	LeftStick = 7,
	RightStick = 8,
	LeftShoulder = 9,
	RightShoulder = 10,
	DPadUp = 11, DPadDown = 12,
	DPadLeft = 13, DPadRight = 14,
	Max
};

struct GamepadState {
	int id = -1;
	bool connected = false;
	float axes[static_cast<int>(GamepadAxis::Max)]{};
	bool buttons[static_cast<int>(GamepadButton::Max)]{};
};

class Input {
public:
	// Config constants
	constexpr static size_t MaxGamepads = 2;
	constexpr static float GamepadDeadzone = 0.15f;

	static void Initialize();
	static void Shutdown();
	static void PollEvents();

	// Keyboard
	static KeyState GetKey(Key key);
	static bool IsKeyDown(Key key) { return GetKey(key) == KeyState::Down; }
	static bool IsKeyPressed(Key key) { return GetKey(key) == KeyState::Pressed; }
	static bool IsKeyReleased(Key key) { return GetKey(key) == KeyState::Released; }

	// Mouse
	static ButtonState GetMouseButton(MouseButton btn);
	static bool IsMouseButtonDown(MouseButton btn) { return GetMouseButton(btn) == ButtonState::Down; }
	static bool IsMouseButtonPressed(MouseButton btn) { return GetMouseButton(btn) == ButtonState::Pressed; }
	static bool IsMouseButtonReleased(MouseButton btn) { return GetMouseButton(btn) == ButtonState::Released; }

	static float GetMouseX();
	static float GetMouseY();
	static Vector2f GetMousePosition() { return { GetMouseX(), GetMouseY() }; }
	static float GetMouseDeltaX();
	static float GetMouseDeltaY();
	static Vector2f GetMouseDelta() { return { GetMouseDeltaX(), GetMouseDeltaY() }; }
	static float GetScrollX();
	static float GetScrollY();
	static Vector2f GetScroll() { return { GetScrollX(), GetScrollY() }; }

	// Gamepad
	static const std::vector<GamepadState>& GetGamepads();
	static int GetGamepadCount();
	static bool IsGamepadConnected(int index = 0);

	static bool IsGamepadButtonDown(GamepadButton btn, int index = 0);
	static bool IsGamepadButtonPressed(GamepadButton btn, int index = 0);
	static bool IsGamepadButtonReleased(GamepadButton btn, int index = 0);

	static float GetGamepadAxis(GamepadAxis axis, int index = 0);
	static Vector2f GetGamepadLeftStick(int index = 0);
	static Vector2f GetGamepadRightStick(int index = 0);

	static bool ShouldQuit();

private:
	struct Impl;
	static std::unique_ptr<Impl> impl;
};
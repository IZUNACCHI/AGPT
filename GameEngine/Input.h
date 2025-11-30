#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "Vec2.h"

enum class Key {
	Unknown = 0,
	A, B, C, D, E, F, G,
	H, I, J, K, L, M, N,
	O, P, Q, R, S, T, U,
	V, W, X, Y, Z,
	Num0, Num1, Num2, Num3, Num4,
	Num5, Num6, Num7, Num8, Num9,
	Space,
	Enter,
	Escape,
	LeftShift, RightShift,
	LeftCtrl, RightCtrl,
	LeftAlt, RightAlt,
	Up, Down, Left, Right,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, Max
};

enum class MouseButton {
	Left = 0,
	Right,
	Middle,
	X1,
	X2,
	Max
};

enum class KeyState {
	Up,
	Down,
	Pressed,
	Released
};

enum class ButtonState {
	Up,
	Down,
	Pressed,
	Released
};

// Gamepad support
enum class GamepadAxis : int {
	Invalid = -1,
	LeftX = 0,
	LeftY = 1,
	RightX = 2,
	RightY = 3,
	LeftTrigger = 4,
	RightTrigger = 5,
	Max
};

enum class GamepadButton : int {
	Invalid = -1,
	South = 0,              // Cross (PS)
	East = 1,              // Circle
	West = 2,              // Square
	North = 3,              // Triangle
	Back = 4,           // View/Share
	Guide = 5,          // Xbox/PS button
	Start = 6,          // Menu/Options
	LeftStick = 7,
	RightStick = 8,
	LeftShoulder = 9,   // LB/L1
	RightShoulder = 10, // RB/R1
	DPadUp = 11,
	DPadDown = 12,
	DPadLeft = 13,
	DPadRight = 14,
	Max
};

struct GamepadState {
	int id = -1;
	bool connected = false;

	// Axis values: sticks [-1..1], triggers [0..1]
	float axes[static_cast<int>(GamepadAxis::Max)]{};

	// Button states
	bool buttons[static_cast<int>(GamepadButton::Max)]{};
};

class Input {
public:
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
	static Vec2 GetMousePosition() { return { GetMouseX(), GetMouseY() }; }
	static float GetMouseDeltaX();
	static float GetMouseDeltaY();
	static Vec2 GetMouseDelta() { return { GetMouseDeltaX(), GetMouseDeltaY() }; }
	static float GetScrollX();
	static float GetScrollY();
	static Vec2 GetScroll() { return { GetScrollX(), GetScrollY() }; }

	// Gamepad
	static const std::vector<GamepadState>& GetGamepads();
	static int GetGamepadCount();

	static bool IsGamepadConnected(int gamepadIndex = 0);

	static bool IsGamepadButtonDown(GamepadButton button, int gamepadIndex = 0);
	static bool IsGamepadButtonPressed(GamepadButton button, int gamepadIndex = 0);
	static bool IsGamepadButtonReleased(GamepadButton button, int gamepadIndex = 0);

	static float GetGamepadAxis(GamepadAxis axis, int gamepadIndex = 0);
	static Vec2 GetGamepadLeftStick(int gamepadIndex = 0);
	static Vec2 GetGamepadRightStick(int gamepadIndex = 0);

	static bool ShouldQuit();

private:
	struct Impl;
	static std::unique_ptr<Impl> impl;
};
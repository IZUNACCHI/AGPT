#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

// ENGINE KEY CODES 
enum class Key {
	Unknown,
	A, B, C, D, E, F, G,
	H, I, J, K, L, M, N,
	O, P, Q, R, S, T, U,
	V, W, X, Y, Z,

	Num0, Num1, Num2, Num3, Num4,
	Num5, Num6, Num7, Num8, Num9,

	Space,
	Enter,
	Escape,
	LeftShift,
	LeftCtrl,
	LeftAlt,
	RightShift,
	RightCtrl,
	RightAlt,

	Up,
	Down,
	Left,
	Right
};

enum class MouseButton {
	Left,
	Right,
	Middle,
	X1,
	X2
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


// GAMEPAD
struct GamepadState {
	int id; // internal engine id
	bool connected = false; 

	std::unordered_map<int, float> axes;     // normalized [-1..1]
	std::unordered_map<int, bool> buttons;   // bool
	std::unordered_map<int, bool> prevButtons;
};

class Input {
public:
	static void Initialize();
	static void Shutdown();
	static void PollEvents();

	// keyboard
	static KeyState GetKey(Key key);
	static bool IsKeyDown(Key key);
	static bool IsKeyPressed(Key key);
	static bool IsKeyReleased(Key key);

	// mouse
	static ButtonState GetMouseButton(MouseButton btn);
	static float GetMouseX();
	static float GetMouseY();
	static float GetMouseDeltaX();
	static float GetMouseDeltaY();
	static float GetScrollX();
	static float GetScrollY();

	// gamepad
	static const std::vector<GamepadState>& GetGamepads();
	static const GamepadState* GetGamepad(int id);

	static bool Close();
private:
	struct Impl;
	static std::unique_ptr<Impl> impl;
};


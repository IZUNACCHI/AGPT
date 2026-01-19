#include "Input.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <ranges>
#include <string>
#include "EngineException.hpp"

std::unique_ptr<Input::Impl> Input::impl = nullptr;

struct Input::Impl {
	// Keyboard
	std::array<bool, static_cast<size_t>(Key::Max)> prevKeyState{};
	std::array<bool, static_cast<size_t>(Key::Max)> currKeyState{};

	// Mouse
	std::array<bool, static_cast<size_t>(MouseButton::Max)> prevMouse{};
	std::array<bool, static_cast<size_t>(MouseButton::Max)> currMouse{};

	float mouseX = 0.f, mouseY = 0.f;
	float lastX = 0.f, lastY = 0.f;
	float scrollX = 0.f, scrollY = 0.f;

	// Gamepad
	struct Pad {
		int id = -1;
		SDL_Gamepad* sdlPad = nullptr;
		bool connected = false;

		std::array<float, static_cast<size_t>(GamepadAxis::Max)> axes{};
		std::array<float, static_cast<size_t>(GamepadAxis::Max)> prevAxes{};
		std::array<bool, static_cast<size_t>(GamepadButton::Max)> buttons{};
		std::array<bool, static_cast<size_t>(GamepadButton::Max)> prevButtons{};
	};

	std::array<Pad, Input::MaxGamepads> pads{};
	bool quitRequested = false;
};

// Translate SDL_Scancode to Key enum
static Key TranslateKey(SDL_Scancode sc) {
	switch (sc) {
		// letters
	case SDL_SCANCODE_A: return Key::A; case SDL_SCANCODE_B: return Key::B;
	case SDL_SCANCODE_C: return Key::C; case SDL_SCANCODE_D: return Key::D;
	case SDL_SCANCODE_E: return Key::E; case SDL_SCANCODE_F: return Key::F;
	case SDL_SCANCODE_G: return Key::G; case SDL_SCANCODE_H: return Key::H;
	case SDL_SCANCODE_I: return Key::I; case SDL_SCANCODE_J: return Key::J;
	case SDL_SCANCODE_K: return Key::K; case SDL_SCANCODE_L: return Key::L;
	case SDL_SCANCODE_M: return Key::M; case SDL_SCANCODE_N: return Key::N;
	case SDL_SCANCODE_O: return Key::O; case SDL_SCANCODE_P: return Key::P;
	case SDL_SCANCODE_Q: return Key::Q; case SDL_SCANCODE_R: return Key::R;
	case SDL_SCANCODE_S: return Key::S; case SDL_SCANCODE_T: return Key::T;
	case SDL_SCANCODE_U: return Key::U; case SDL_SCANCODE_V: return Key::V;
	case SDL_SCANCODE_W: return Key::W; case SDL_SCANCODE_X: return Key::X;
	case SDL_SCANCODE_Y: return Key::Y; case SDL_SCANCODE_Z: return Key::Z;
		// numbers
	case SDL_SCANCODE_0: return Key::Num0; case SDL_SCANCODE_1: return Key::Num1;
	case SDL_SCANCODE_2: return Key::Num2; case SDL_SCANCODE_3: return Key::Num3;
	case SDL_SCANCODE_4: return Key::Num4; case SDL_SCANCODE_5: return Key::Num5;
	case SDL_SCANCODE_6: return Key::Num6; case SDL_SCANCODE_7: return Key::Num7;
	case SDL_SCANCODE_8: return Key::Num8; case SDL_SCANCODE_9: return Key::Num9;
		// common keys
	case SDL_SCANCODE_SPACE: return Key::Space;
	case SDL_SCANCODE_RETURN: return Key::Enter;
	case SDL_SCANCODE_ESCAPE: return Key::Escape;
	case SDL_SCANCODE_LSHIFT: return Key::LeftShift;
	case SDL_SCANCODE_RSHIFT: return Key::RightShift;
	case SDL_SCANCODE_LCTRL: return Key::LeftCtrl;
	case SDL_SCANCODE_RCTRL: return Key::RightCtrl;
	case SDL_SCANCODE_LALT: return Key::LeftAlt;
	case SDL_SCANCODE_RALT: return Key::RightAlt;
	case SDL_SCANCODE_UP: return Key::Up;
	case SDL_SCANCODE_DOWN: return Key::Down;
	case SDL_SCANCODE_LEFT: return Key::Left;
	case SDL_SCANCODE_RIGHT: return Key::Right;
		// function keys
	case SDL_SCANCODE_F1:  return Key::F1;  case SDL_SCANCODE_F2:  return Key::F2;
	case SDL_SCANCODE_F3:  return Key::F3;  case SDL_SCANCODE_F4:  return Key::F4;
	case SDL_SCANCODE_F5:  return Key::F5;  case SDL_SCANCODE_F6:  return Key::F6;
	case SDL_SCANCODE_F7:  return Key::F7;  case SDL_SCANCODE_F8:  return Key::F8;
	case SDL_SCANCODE_F9:  return Key::F9;  case SDL_SCANCODE_F10: return Key::F10;
	case SDL_SCANCODE_F11: return Key::F11; case SDL_SCANCODE_F12: return Key::F12;

		// other keys
	case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
	case SDL_SCANCODE_TAB: return Key::Tab;
	case SDL_SCANCODE_CAPSLOCK: return Key::Capslock;
	case SDL_SCANCODE_MINUS: return Key::Minus;
	case SDL_SCANCODE_EQUALS: return Key::Equals;
	case SDL_SCANCODE_LEFTBRACKET: return Key::LeftBracket;
	case SDL_SCANCODE_RIGHTBRACKET: return Key::RightBracket;
	case SDL_SCANCODE_BACKSLASH: return Key::Backslash;
	case SDL_SCANCODE_SEMICOLON: return Key::Semicolon;
	case SDL_SCANCODE_APOSTROPHE: return Key::Apostrophe;
	case SDL_SCANCODE_GRAVE: return Key::Grave;
	case SDL_SCANCODE_COMMA: return Key::Comma;
	case SDL_SCANCODE_PERIOD: return Key::Period;
	case SDL_SCANCODE_SLASH: return Key::Slash;
		
	case SDL_SCANCODE_NUMLOCKCLEAR: return Key::Numlock;
	case SDL_SCANCODE_SCROLLLOCK: return Key::Scrolllock;
	case SDL_SCANCODE_PRINTSCREEN: return Key::Printscreen;
	case SDL_SCANCODE_INSERT: return Key::Insert;
	case SDL_SCANCODE_DELETE: return Key::Delete;
	case SDL_SCANCODE_HOME: return Key::Home;
	case SDL_SCANCODE_END: return Key::End;
	case SDL_SCANCODE_PAGEUP: return Key::PageUp;
	case SDL_SCANCODE_PAGEDOWN: return Key::PageDown;

	case SDL_SCANCODE_KP_0: return Key::KP0; case SDL_SCANCODE_KP_1: return Key::KP1;
	case SDL_SCANCODE_KP_2: return Key::KP2; case SDL_SCANCODE_KP_3: return Key::KP3;
	case SDL_SCANCODE_KP_4: return Key::KP4; case SDL_SCANCODE_KP_5: return Key::KP5;
	case SDL_SCANCODE_KP_6: return Key::KP6; case SDL_SCANCODE_KP_7: return Key::KP7;
	case SDL_SCANCODE_KP_8: return Key::KP8; case SDL_SCANCODE_KP_9: return Key::KP9;
	case SDL_SCANCODE_KP_DECIMAL: return Key::KPPeriod;
	case SDL_SCANCODE_KP_DIVIDE: return Key::KPDivide;
	case SDL_SCANCODE_KP_MULTIPLY: return Key::KPMultiply;
	case SDL_SCANCODE_KP_MINUS: return Key::KPMinus;
	case SDL_SCANCODE_KP_PLUS: return Key::KPPlus;
	case SDL_SCANCODE_KP_ENTER: return Key::KPEnter;

	default: return Key::Unknown;
	}
}

// Translate SDL mouse button to MouseButton enum
static MouseButton TranslateMouse(Uint8 btn) {
	switch (btn) {
	case SDL_BUTTON_LEFT:   return MouseButton::Left;
	case SDL_BUTTON_RIGHT:  return MouseButton::Right;
	case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
		// extra buttons
	case SDL_BUTTON_X1:     return MouseButton::X1;
	case SDL_BUTTON_X2:     return MouseButton::X2;
	default:                return MouseButton::Invalid;
	}
}

// lifecycle
void Input::Initialize() {
	impl = std::make_unique<Impl>();
	if(!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
		THROW_ENGINE_EXCEPTION("Failed to initialize SDL Gamepad subsystem: " + std::string(SDL_GetError()));
	}
	
}

void Input::Shutdown() {
	for (auto& pad : impl->pads) {
		if (pad.sdlPad) SDL_CloseGamepad(pad.sdlPad);
	}
	impl.reset();
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

// Event polling
void Input::PollEvents() {
	impl->prevKeyState = impl->currKeyState;
	impl->prevMouse = impl->currMouse;

	for (auto& pad : impl->pads) {
		if (pad.connected) {
			pad.prevAxes = pad.axes;
			pad.prevButtons = pad.buttons;
		}
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:                    
			impl->quitRequested = true; break;

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP: {
			Key k = TranslateKey(event.key.scancode);
			if (k != Key::Unknown)
				impl->currKeyState[static_cast<size_t>(k)] = (event.type == SDL_EVENT_KEY_DOWN);
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			MouseButton mb = TranslateMouse(event.button.button);
			if (mb != MouseButton::Invalid)
				impl->currMouse[static_cast<size_t>(mb)] = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
			break;
		}

		case SDL_EVENT_MOUSE_MOTION: {
			impl->lastX = impl->mouseX;
			impl->lastY = impl->mouseY;
			impl->mouseX = static_cast<float>(event.motion.x);
			impl->mouseY = static_cast<float>(event.motion.y);
			break;
		}

		case SDL_EVENT_MOUSE_WHEEL: {
			impl->scrollX += static_cast<float>(event.wheel.x);
			impl->scrollY += static_cast<float>(event.wheel.y);
			break;
		}

		
		case SDL_EVENT_GAMEPAD_ADDED: {
			for (size_t i = 0; i < Input::MaxGamepads; ++i) {
				if (impl->pads[i].id == -1) {
					impl->pads[i].id = event.gdevice.which;
					impl->pads[i].sdlPad = SDL_OpenGamepad(event.gdevice.which);
					impl->pads[i].connected = (impl->pads[i].sdlPad != nullptr);
					break;
				}
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_REMOVED: {
			for (auto& pad : impl->pads) {
				if (pad.id == event.gdevice.which) {
					if (pad.sdlPad) SDL_CloseGamepad(pad.sdlPad);
					pad = Impl::Pad{};  // resets everything cleanly
					break;
				}
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP: {
			auto it = std::ranges::find_if(impl->pads,
				[id = event.gbutton.which](const auto& p) { return p.id == id && p.connected; });
			if (it != impl->pads.end())
				it->buttons[event.gbutton.button] = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
			break;
		}

		case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
			auto it = std::ranges::find_if(impl->pads,
				[id = event.gaxis.which](const auto& p) { return p.id == id && p.connected; });
			if (it != impl->pads.end()) {
				float val = (event.gaxis.value == -32768) ? -1.f : event.gaxis.value / 32767.f;
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ||
					event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
					val = val * 0.5f + 0.5f; // 0..1 instead of -1..1
				it->axes[event.gaxis.axis] = val;
			}
			break;
		}
		}
	}
}

// Query functions

// keyboard
KeyState Input::GetKey(Key k) {
	size_t i = static_cast<size_t>(k);
	if (i >= impl->currKeyState.size()) return KeyState::Up;
	bool prev = impl->prevKeyState[i];
	bool cur = impl->currKeyState[i];
	if (cur && !prev) return KeyState::Pressed;
	if (!cur && prev) return KeyState::Released;
	return cur ? KeyState::Down : KeyState::Up;
}

// mouse
ButtonState Input::GetMouseButton(MouseButton btn) {
	if (btn == MouseButton::Invalid) return ButtonState::Up;
	size_t i = static_cast<size_t>(btn);
	if (i >= impl->currMouse.size()) return ButtonState::Up;
	bool prev = impl->prevMouse[i];
	bool cur = impl->currMouse[i];
	if (cur && !prev) return ButtonState::Pressed;
	if (!cur && prev) return ButtonState::Released;
	return cur ? ButtonState::Down : ButtonState::Up;
}

// mouse position
float Input::GetMouseX() { return impl->mouseX; }
float Input::GetMouseY() { return impl->mouseY; }
// mouse delta
float Input::GetMouseDeltaX() { return impl->mouseX - impl->lastX; }
float Input::GetMouseDeltaY() { return impl->mouseY - impl->lastY; }

// scroll
float Input::GetScrollX() { float s = impl->scrollX; impl->scrollX = 0.f; return s; }
float Input::GetScrollY() { float s = impl->scrollY; impl->scrollY = 0.f; return s; }

// gamepad
const std::vector<GamepadState>& Input::GetGamepads() {
	static std::vector<GamepadState> list;
	list.clear();
	list.reserve(Input::MaxGamepads);
	for (const auto& p : impl->pads) {
		if (p.connected) {
			GamepadState st;
			st.id = p.id;
			st.connected = true;
			std::copy(p.axes.begin(), p.axes.end(), st.axes);
			std::copy(p.buttons.begin(), p.buttons.end(), st.buttons);
			list.push_back(st);
		}
	}
	return list;
}

int Input::GetGamepadCount() {
	return static_cast<int>(std::ranges::count_if(impl->pads, [](const auto& p) { return p.connected; }));
}

bool Input::IsGamepadConnected(int index) {
	return index >= 0 && static_cast<size_t>(index) < Input::MaxGamepads && impl->pads[index].connected;
}

bool Input::IsGamepadButtonDown(GamepadButton b, int index) {
	if (b == GamepadButton::Invalid || !IsGamepadConnected(index)) return false;
	return impl->pads[index].buttons[static_cast<size_t>(b)];
}

bool Input::IsGamepadButtonPressed(GamepadButton b, int index) {
	if (b == GamepadButton::Invalid || !IsGamepadConnected(index)) return false;
	size_t i = static_cast<size_t>(b);
	return impl->pads[index].buttons[i] && !impl->pads[index].prevButtons[i];
}

bool Input::IsGamepadButtonReleased(GamepadButton b, int index) {
	if (b == GamepadButton::Invalid || !IsGamepadConnected(index)) return false;
	size_t i = static_cast<size_t>(b);
	return !impl->pads[index].buttons[i] && impl->pads[index].prevButtons[i];
}

float Input::GetGamepadAxis(GamepadAxis a, int index) {
	if (a == GamepadAxis::Invalid || !IsGamepadConnected(index)) return 0.f;
	float v = impl->pads[index].axes[static_cast<size_t>(a)];
	if ((a == GamepadAxis::LeftX || a == GamepadAxis::LeftY ||
		a == GamepadAxis::RightX || a == GamepadAxis::RightY) &&
		std::abs(v) < Input::GamepadDeadzone)
		return 0.f;
	return v;
}

Vector2f Input::GetGamepadLeftStick(int index) { return { GetGamepadAxis(GamepadAxis::LeftX,  index), GetGamepadAxis(GamepadAxis::LeftY,  index) }; }
Vector2f Input::GetGamepadRightStick(int index) { return { GetGamepadAxis(GamepadAxis::RightX, index), GetGamepadAxis(GamepadAxis::RightY, index) }; }

bool Input::ShouldQuit() { return impl->quitRequested; }
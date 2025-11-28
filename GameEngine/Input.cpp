#include "Input.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <ranges>

struct Input::Impl {
	// Keyboard
	std::unordered_map<Key, bool> prevKeyState;
	std::unordered_map<Key, bool> currKeyState;

	// Mouse
	std::unordered_map<MouseButton, bool> prevMouse;
	std::unordered_map<MouseButton, bool> currMouse;

	float mouseX = 0.f;
	float mouseY = 0.f;
	float lastX = 0.f;
	float lastY = 0.f;
	float scrollX = 0.f;
	float scrollY = 0.f;

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

	std::vector<Pad> pads;

	bool quitRequested = false;
};

// ------------------------------
// HELPERS
// ------------------------------
static Key TranslateKey(SDL_Keycode key) {
	switch (key) {
	case SDLK_a: case SDLK_A: return Key::A;
	case SDLK_b: case SDLK_B: return Key::B;
	case SDLK_c: case SDLK_C: return Key::C;
	case SDLK_d: case SDLK_D: return Key::D;
	case SDLK_e: case SDLK_E: return Key::E;
	case SDLK_f: case SDLK_F: return Key::F;
	case SDLK_g: case SDLK_G: return Key::G;
	case SDLK_h: case SDLK_H: return Key::H;
	case SDLK_i: case SDLK_I: return Key::I;
	case SDLK_j: case SDLK_J: return Key::J;
	case SDLK_k: case SDLK_K: return Key::K;
	case SDLK_l: case SDLK_L: return Key::L;
	case SDLK_m: case SDLK_M: return Key::M;
	case SDLK_n: case SDLK_N: return Key::N;
	case SDLK_o: case SDLK_O: return Key::O;
	case SDLK_p: case SDLK_P: return Key::P;
	case SDLK_q: case SDLK_Q: return Key::Q;
	case SDLK_r: case SDLK_R: return Key::R;
	case SDLK_s: case SDLK_S: return Key::S;
	case SDLK_t: case SDLK_T: return Key::T;
	case SDLK_u: case SDLK_U: return Key::U;
	case SDLK_v: case SDLK_V: return Key::V;
	case SDLK_w: case SDLK_W: return Key::W;
	case SDLK_x: case SDLK_X: return Key::X;
	case SDLK_y: case SDLK_Y: return Key::Y;
	case SDLK_z: case SDLK_Z: return Key::Z;
	case SDLK_0: return Key::Num0;
	case SDLK_1: return Key::Num1;
	case SDLK_2: return Key::Num2;
	case SDLK_3: return Key::Num3;
	case SDLK_4: return Key::Num4;
	case SDLK_5: return Key::Num5;
	case SDLK_6: return Key::Num6;
	case SDLK_7: return Key::Num7;
	case SDLK_8: return Key::Num8;
	case SDLK_9: return Key::Num9;
	case SDLK_SPACE: return Key::Space;
	case SDLK_RETURN: return Key::Enter;
	case SDLK_ESCAPE: return Key::Escape;
	case SDLK_LSHIFT: return Key::LeftShift;
	case SDLK_RSHIFT: return Key::RightShift;
	case SDLK_LCTRL: return Key::LeftCtrl;
	case SDLK_RCTRL: return Key::RightCtrl;
	case SDLK_LALT: return Key::LeftAlt;
	case SDLK_RALT: return Key::RightAlt;
	case SDLK_UP: return Key::Up;
	case SDLK_DOWN: return Key::Down;
	case SDLK_LEFT: return Key::Left;
	case SDLK_RIGHT: return Key::Right;
	case SDLK_F1: return Key::F1;
	case SDLK_F2: return Key::F2;
	case SDLK_F3: return Key::F3;
	case SDLK_F4: return Key::F4;
	case SDLK_F5: return Key::F5;
	case SDLK_F6: return Key::F6;
	case SDLK_F7: return Key::F7;
	case SDLK_F8: return Key::F8;
	case SDLK_F9: return Key::F9;
	case SDLK_F10: return Key::F10;
	case SDLK_F11: return Key::F11;
	case SDLK_F12: return Key::F12;
	default: return Key::Unknown;
	}
}

static MouseButton TranslateMouse(Uint8 btn) {
	switch (btn) {
	case SDL_BUTTON_LEFT: return MouseButton::Left;
	case SDL_BUTTON_RIGHT: return MouseButton::Right;
	case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
	case SDL_BUTTON_X1: return MouseButton::X1;
	case SDL_BUTTON_X2: return MouseButton::X2;
	default: return MouseButton::X2; // invalid
	}
}

// ------------------------------
// INITIALIZATION
// ------------------------------
void Input::Initialize() {
	impl = std::make_unique<Impl>();
	SDL_InitSubSystem(SDL_INIT_GAMEPAD);
}

void Input::Shutdown() {
	for (auto& pad : impl->pads) {
		if (pad.sdlPad) SDL_CloseGamepad(pad.sdlPad);
	}
	impl.reset();
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

// ------------------------------
// MAIN EVENT POLLING
// ------------------------------
void Input::PollEvents() {
	// Store previous states
	impl->prevKeyState = impl->currKeyState;
	impl->prevMouse = impl->currMouse;

	for (auto& pad : impl->pads) {
		pad.prevAxes = pad.axes;
		pad.prevButtons = pad.buttons;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			impl->quitRequested = true;
			break;

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP: {
			Key k = TranslateKey(event.key.keysym.sym);
			if (k != Key::Unknown) {
				impl->currKeyState[k] = (event.type == SDL_EVENT_KEY_DOWN);
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			MouseButton mb = TranslateMouse(event.button.button);
			if (mb != MouseButton::X2) {
				impl->currMouse[mb] = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
			}
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

								  // Gamepad events
		case SDL_EVENT_GAMEPAD_ADDED: {
			Impl::Pad newPad;
			newPad.id = event.gdevice.which;
			newPad.sdlPad = SDL_OpenGamepad(event.gdevice.which);
			newPad.connected = (newPad.sdlPad != nullptr);
			if (newPad.connected) {
				impl->pads.push_back(std::move(newPad));
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_REMOVED: {
			auto it = std::ranges::find_if(impl->pads, [which = event.gdevice.which](const Impl::Pad& p) {
				return p.id == which;
				});
			if (it != impl->pads.end()) {
				SDL_CloseGamepad(it->sdlPad);
				impl->pads.erase(it);
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP: {
			auto it = std::ranges::find_if(impl->pads, [which = event.gbutton.which](const Impl::Pad& p) {
				return p.id == which && p.connected;
				});
			if (it != impl->pads.end() && event.gbutton.button < static_cast<int>(GamepadButton::Max)) {
				it->buttons[event.gbutton.button] = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
			auto it = std::ranges::find_if(impl->pads, [which = event.gaxis.which](const Impl::Pad& p) {
				return p.id == which && p.connected;
				});
			if (it != impl->pads.end() && event.gaxis.axis < static_cast<int>(GamepadAxis::Max)) {
				Sint16 raw = event.gaxis.value;
				float normalized;
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTTRIGGER ||
					event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTTRIGGER) {
					normalized = raw / 32767.0f; // 0 to 1
				}
				else {
					normalized = (raw == SDL_MIN_SINT16) ? -1.0f : static_cast<float>(raw) / 32767.0f; // -1 to 1
				}
				it->axes[event.gaxis.axis] = normalized;
			}
			break;
		}
		}
	}
}

// ------------------------------
// KEYBOARD
// ------------------------------
KeyState Input::GetKey(Key k) {
	bool prev = impl->prevKeyState.contains(k) ? impl->prevKeyState[k] : false;
	bool cur = impl->currKeyState.contains(k) ? impl->currKeyState[k] : false;

	if (cur && !prev) return KeyState::Pressed;
	if (!cur && prev) return KeyState::Released;
	if (cur) return KeyState::Down;
	return KeyState::Up;
}

// ------------------------------
// MOUSE
// ------------------------------
ButtonState Input::GetMouseButton(MouseButton b) {
	bool prev = impl->prevMouse.contains(b) ? impl->prevMouse[b] : false;
	bool cur = impl->currMouse.contains(b) ? impl->currMouse[b] : false;

	if (cur && !prev) return ButtonState::Pressed;
	if (!cur && prev) return ButtonState::Released;
	if (cur) return ButtonState::Down;
	return ButtonState::Up;
}

float Input::GetMouseX() { return impl->mouseX; }
float Input::GetMouseY() { return impl->mouseY; }
float Input::GetMouseDeltaX() { return impl->mouseX - impl->lastX; }
float Input::GetMouseDeltaY() { return impl->mouseY - impl->lastY; }

float Input::GetScrollX() {
	float s = impl->scrollX;
	impl->scrollX = 0.f;
	return s;
}
float Input::GetScrollY() {
	float s = impl->scrollY;
	impl->scrollY = 0.f;
	return s;
}

// ------------------------------
// GAMEPAD
// ------------------------------
const std::vector<GamepadState>& Input::GetGamepads() {
	static std::vector<GamepadState> exposed;
	exposed.clear();

	for (const auto& p : impl->pads) {
		if (!p.connected) continue;

		GamepadState st;
		st.id = p.id;
		st.connected = true;
		std::copy(p.axes.begin(), p.axes.end(), st.axes);
		std::copy(p.buttons.begin(), p.buttons.end(), st.buttons);
		exposed.push_back(st);
	}
	return exposed;
}

int Input::GetGamepadCount() {
	return std::ranges::count_if(impl->pads, [](const Impl::Pad& p) { return p.connected; });
}

bool Input::IsGamepadConnected(int index) {
	if (index < 0 || static_cast<size_t>(index) >= impl->pads.size()) return false;
	return impl->pads[index].connected;
}

bool Input::IsGamepadButtonDown(GamepadButton button, int index) {
	if (button == GamepadButton::Invalid) return false;
	if (index < 0 || static_cast<size_t>(index) >= impl->pads.size()) return false;
	const auto& pad = impl->pads[index];
	if (!pad.connected) return false;
	return pad.buttons[static_cast<size_t>(button)];
}

bool Input::IsGamepadButtonPressed(GamepadButton button, int index) {
	if (button == GamepadButton::Invalid) return false;
	if (index < 0 || static_cast<size_t>(index) >= impl->pads.size()) return false;
	const auto& pad = impl->pads[index];
	if (!pad.connected) return false;
	size_t idx = static_cast<size_t>(button);
	return pad.buttons[idx] && !pad.prevButtons[idx];
}

bool Input::IsGamepadButtonReleased(GamepadButton button, int index) {
	if (button == GamepadButton::Invalid) return false;
	if (index < 0 || static_cast<size_t>(index) >= impl->pads.size()) return false;
	const auto& pad = impl->pads[index];
	if (!pad.connected) return false;
	size_t idx = static_cast<size_t>(button);
	return !pad.buttons[idx] && pad.prevButtons[idx];
}

float Input::GetGamepadAxis(GamepadAxis axis, int index) {
	if (axis == GamepadAxis::Invalid) return 0.0f;
	if (index < 0 || static_cast<size_t>(index) >= impl->pads.size()) return 0.0f;
	const auto& pad = impl->pads[index];
	if (!pad.connected) return 0.0f;

	constexpr float deadzone = 0.15f;
	size_t idx = static_cast<size_t>(axis);
	float value = pad.axes[idx];

	// Deadzone only for sticks
	if ((axis == GamepadAxis::LeftX || axis == GamepadAxis::LeftY ||
		axis == GamepadAxis::RightX || axis == GamepadAxis::RightY) &&
		std::abs(value) < deadzone) {
		return 0.0f;
	}

	return value;
}

Vec2 Input::GetGamepadLeftStick(int index) {
	return { GetGamepadAxis(GamepadAxis::LeftX, index), GetGamepadAxis(GamepadAxis::LeftY, index) };
}

Vec2 Input::GetGamepadRightStick(int index) {
	return { GetGamepadAxis(GamepadAxis::RightX, index), GetGamepadAxis(GamepadAxis::RightY, index) };
}

bool Input::Close() {
	return impl->quitRequested;
}
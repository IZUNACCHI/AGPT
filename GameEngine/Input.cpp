#include "Input.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <ranges>
#include <iostream>

std::unique_ptr<Input::Impl> Input::impl = nullptr;

struct Input::Impl {
	// Keyboard
	std::array<bool, static_cast<size_t>(Key::Max)> prevKeyState{};
	std::array<bool, static_cast<size_t>(Key::Max)> currKeyState{};

	// Mouse
	std::array<bool, static_cast<size_t>(MouseButton::Max)> prevMouse{};
	std::array<bool, static_cast<size_t>(MouseButton::Max)> currMouse{};

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

	// Fixed slots so player index never changes when someone plugs/unplugs
	static constexpr size_t MAX_PADS = 16;
	std::array<Pad, MAX_PADS> pads{};

	bool quitRequested = false;
};

// ------------------------------
// HELPERS
// ------------------------------
static Key TranslateKey(SDL_Scancode scancode) {
	switch (scancode) {
	case SDL_SCANCODE_A: return Key::A;
	case SDL_SCANCODE_B: return Key::B;
	case SDL_SCANCODE_C: return Key::C;
	case SDL_SCANCODE_D: return Key::D;
	case SDL_SCANCODE_E: return Key::E;
	case SDL_SCANCODE_F: return Key::F;
	case SDL_SCANCODE_G: return Key::G;
	case SDL_SCANCODE_H: return Key::H;
	case SDL_SCANCODE_I: return Key::I;
	case SDL_SCANCODE_J: return Key::J;
	case SDL_SCANCODE_K: return Key::K;
	case SDL_SCANCODE_L: return Key::L;
	case SDL_SCANCODE_M: return Key::M;
	case SDL_SCANCODE_N: return Key::N;
	case SDL_SCANCODE_O: return Key::O;
	case SDL_SCANCODE_P: return Key::P;
	case SDL_SCANCODE_Q: return Key::Q;
	case SDL_SCANCODE_R: return Key::R;
	case SDL_SCANCODE_S: return Key::S;
	case SDL_SCANCODE_T: return Key::T;
	case SDL_SCANCODE_U: return Key::U;
	case SDL_SCANCODE_V: return Key::V;
	case SDL_SCANCODE_W: return Key::W;
	case SDL_SCANCODE_X: return Key::X;
	case SDL_SCANCODE_Y: return Key::Y;
	case SDL_SCANCODE_Z: return Key::Z;
	case SDL_SCANCODE_0: return Key::Num0;
	case SDL_SCANCODE_1: return Key::Num1;
	case SDL_SCANCODE_2: return Key::Num2;
	case SDL_SCANCODE_3: return Key::Num3;
	case SDL_SCANCODE_4: return Key::Num4;
	case SDL_SCANCODE_5: return Key::Num5;
	case SDL_SCANCODE_6: return Key::Num6;
	case SDL_SCANCODE_7: return Key::Num7;
	case SDL_SCANCODE_8: return Key::Num8;
	case SDL_SCANCODE_9: return Key::Num9;
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
	case SDL_SCANCODE_F1: return Key::F1;
	case SDL_SCANCODE_F2: return Key::F2;
	case SDL_SCANCODE_F3: return Key::F3;
	case SDL_SCANCODE_F4: return Key::F4;
	case SDL_SCANCODE_F5: return Key::F5;
	case SDL_SCANCODE_F6: return Key::F6;
	case SDL_SCANCODE_F7: return Key::F7;
	case SDL_SCANCODE_F8: return Key::F8;
	case SDL_SCANCODE_F9: return Key::F9;
	case SDL_SCANCODE_F10: return Key::F10;
	case SDL_SCANCODE_F11: return Key::F11;
	case SDL_SCANCODE_F12: return Key::F12;
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
		if (pad.connected) {
			pad.prevAxes = pad.axes;
			pad.prevButtons = pad.buttons;
		}
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			impl->quitRequested = true;
			break;

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP: {
			Key k = TranslateKey(event.key.scancode);
			if (k != Key::Unknown) {
				size_t idx = static_cast<size_t>(k);
				impl->currKeyState[idx] = (event.type == SDL_EVENT_KEY_DOWN);
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			MouseButton mb = TranslateMouse(event.button.button);
			if (!(MouseButton::Left > mb && mb > MouseButton::Max)) {
				size_t idx = static_cast<size_t>(mb);
				impl->currMouse[idx] = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
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
			SDL_JoystickID id = event.gdevice.which;
			// find free slot
			for (size_t i = 0; i < Impl::MAX_PADS; ++i) {
				if (impl->pads[i].id == -1) {
					impl->pads[i].id = id;
					impl->pads[i].sdlPad = SDL_OpenGamepad(id);
					impl->pads[i].connected = (impl->pads[i].sdlPad != nullptr);
					if (impl->pads[i].connected) std::cout << "Gamepad " << i << " connected\n";
					break;
				}
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_REMOVED: {
			for (auto& pad : impl->pads) {
				if (pad.id == event.gdevice.which) {
					if (pad.sdlPad) SDL_CloseGamepad(pad.sdlPad);
					pad.id =-1; // reset
					break;
				}
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
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
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
	size_t idx = static_cast<size_t>(k);
	if (idx >= impl->currKeyState.size()) return KeyState::Up;

	bool prev = impl->prevKeyState[idx];
	bool curr = impl->currKeyState[idx];

	if (curr && !prev) return KeyState::Pressed;
	if (!curr && prev) return KeyState::Released;
	return curr ? KeyState::Down : KeyState::Up;
}

// ------------------------------
// MOUSE
// ------------------------------
ButtonState Input::GetMouseButton(MouseButton btn) {
	size_t idx = static_cast<size_t>(btn);
	if (idx >= impl->currMouse.size()) return ButtonState::Up;

	bool prev = impl->prevMouse[idx];
	bool curr = impl->currMouse[idx];

	if (curr && !prev) return ButtonState::Pressed;
	if (!curr && prev) return ButtonState::Released;
	return curr ? ButtonState::Down : ButtonState::Up;
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

// GAMEPAD
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
    return static_cast<int>(std::ranges::count_if(impl->pads, [](const Impl::Pad& p) { return p.connected; }));
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

bool Input::ShouldQuit() {
	return impl->quitRequested;
}
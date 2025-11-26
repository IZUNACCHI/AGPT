#include "Input.h"
#include <SDL3/SDL.h>
#include <algorithm>

// ------------------------------
// INTERNAL IMPLEMENTATION
// ------------------------------
struct Input::Impl
{
	// key states
	std::unordered_map<Key, bool> prevKeyState;
	std::unordered_map<Key, bool> currKeyState;

	// mouse
	std::unordered_map<MouseButton, bool> prevMouse;
	std::unordered_map<MouseButton, bool> currMouse;

	float mouseX = 0.f;
	float mouseY = 0.f;
	float lastX = 0.f;
	float lastY = 0.f;
	float scrollX = 0.f;
	float scrollY = 0.f;

	// gamepads
	struct Pad {
		int id;
		SDL_Gamepad* sdlPad = nullptr;
		bool connected = false;

		std::unordered_map<int, float> axes;
		std::unordered_map<int, float> prevAxes;

		std::unordered_map<int, bool> buttons;
		std::unordered_map<int, bool> prevButtons;
	};

	std::vector<Pad> pads;

	bool quitRequested = false;

	// --- SDL keycode to engine Key ---
	Key translateKey(SDL_Keycode key)
	{
		switch (key)
		{
		case SDLK_A: return Key::A;
		case SDLK_D: return Key::D;
		case SDLK_W: return Key::W;
		case SDLK_S: return Key::S;
		case SDLK_ESCAPE: return Key::Escape;
		case SDLK_SPACE: return Key::Space;
		case SDLK_UP: return Key::Up;
		case SDLK_DOWN: return Key::Down;
		case SDLK_LEFT: return Key::Left;
		case SDLK_RIGHT: return Key::Right;
		default: return Key::Unknown;
		}
	}

	MouseButton translateMouse(Uint8 btn)
	{
		switch (btn)
		{
		case SDL_BUTTON_LEFT: return MouseButton::Left;
		case SDL_BUTTON_RIGHT: return MouseButton::Right;
		case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
		case SDL_BUTTON_X1: return MouseButton::X1;
		case SDL_BUTTON_X2: return MouseButton::X2;
		default: return MouseButton::Left;
		}
	}

	// gamepad hotplug
	void handleGamepadAdded(int which)
	{
		SDL_Gamepad* gp = SDL_OpenGamepad(which);
		if (!gp) return;

		Pad pad;
		pad.id = which;
		pad.connected = true;
		pad.sdlPad = gp;

		pads.push_back(pad);
	}

	void handleGamepadRemoved(int which)
	{
		for (auto& p : pads)
		{
			if (p.id == which)
			{
				p.connected = false;
				if (p.sdlPad)
				{
					SDL_CloseGamepad(p.sdlPad);
					p.sdlPad = nullptr;
				}
			}
		}
	}
};


std::unique_ptr<Input::Impl> Input::impl = nullptr;

// ------------------------------
// PUBLIC API IMPLEMENTATION
// ------------------------------

void Input::Initialize()
{
	impl = std::make_unique<Impl>();
}

void Input::Shutdown()
{
	impl.reset();
}

void Input::PollEvents()
{
	// Store previous states
	impl->prevKeyState = impl->currKeyState;
	impl->prevMouse = impl->currMouse;
	impl->scrollX = impl->scrollY = 0.f;

	impl->lastX = impl->mouseX;
	impl->lastY = impl->mouseY;

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_EVENT_QUIT: {
			impl->quitRequested = true;
			break;
		}

		case SDL_EVENT_KEY_DOWN:
		{
			Key k = impl->translateKey(e.key.key);
			impl->currKeyState[k] = true;
			break;
		}
		case SDL_EVENT_KEY_UP:
		{
			Key k = impl->translateKey(e.key.key);
			impl->currKeyState[k] = false;
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
			impl->mouseX = (float)e.motion.x;
			impl->mouseY = (float)e.motion.y;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			impl->currMouse[impl->translateMouse(e.button.button)] = true;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			impl->currMouse[impl->translateMouse(e.button.button)] = false;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			impl->scrollX = e.wheel.x;
			impl->scrollY = e.wheel.y;
			break;

		case SDL_EVENT_GAMEPAD_ADDED:
			impl->handleGamepadAdded(e.gdevice.which);
			break;

		case SDL_EVENT_GAMEPAD_REMOVED:
			impl->handleGamepadRemoved(e.gdevice.which);
			break;

		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		{
			for (auto& p : impl->pads)
			{
				if (p.id == e.gaxis.which)
				{
					float v = e.gaxis.value / 32767.0f;
					p.axes[e.gaxis.axis] = std::clamp(v, -1.f, 1.f);
				}
			}
			break;
		}
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
		{
			bool down = (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
			for (auto& p : impl->pads)
			{
				if (p.id == e.gbutton.which)
					p.buttons[e.gbutton.button] = down;
			}
			break;
		}
		}
	}

	// copy prev states for gamepads
	for (auto& p : impl->pads)
	{
		p.prevButtons = p.buttons;
		p.prevAxes = p.axes;
	}
}

// ------------------------------
// KEYBOARD
// ------------------------------

KeyState Input::GetKey(Key key)
{
	bool prev = impl->prevKeyState[key];
	bool cur = impl->currKeyState[key];

	if (cur && !prev) return KeyState::Pressed;
	if (!cur && prev) return KeyState::Released;
	if (cur) return KeyState::Down;
	return KeyState::Up;
}

bool Input::IsKeyDown(Key k) { return impl->currKeyState[k]; }
bool Input::IsKeyPressed(Key k) { return GetKey(k) == KeyState::Pressed; }
bool Input::IsKeyReleased(Key k) { return GetKey(k) == KeyState::Released; }

// ------------------------------
// MOUSE
// ------------------------------
ButtonState Input::GetMouseButton(MouseButton b)
{
	bool prev = impl->prevMouse[b];
	bool cur = impl->currMouse[b];

	if (cur && !prev) return ButtonState::Pressed;
	if (!cur && prev) return ButtonState::Released;
	if (cur) return ButtonState::Down;
	return ButtonState::Up;
}

float Input::GetMouseX() { return impl->mouseX; }
float Input::GetMouseY() { return impl->mouseY; }

float Input::GetMouseDeltaX() { return impl->mouseX - impl->lastX; }
float Input::GetMouseDeltaY() { return impl->mouseY - impl->lastY; }

float Input::GetScrollX() { return impl->scrollX; }
float Input::GetScrollY() { return impl->scrollY; }

// ------------------------------
// GAMEPADS
// ------------------------------
const std::vector<GamepadState>& Input::GetGamepads()
{
	// Convert internal Pad to public GamepadState
	static std::vector<GamepadState> exposed;
	exposed.clear();

	for (const auto& p : impl->pads)
	{
		GamepadState st;
		st.id = p.id;
		st.connected = p.connected;
		st.axes = p.axes;
		st.buttons = p.buttons;
		st.prevButtons = p.prevButtons;
		exposed.push_back(st);
	}

	return exposed;
}

const GamepadState* Input::GetGamepad(int id)
{
	for (const auto& p : impl->pads)
		if (p.id == id) return &GetGamepads()[&p - impl->pads.data()];
	return nullptr;
}

bool Input::ShouldQuit() {
	return impl->quitRequested;
}
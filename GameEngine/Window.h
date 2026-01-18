#pragma once
#include <string>
#include <memory>
#include "Types.hpp"

namespace Engine {

	struct WindowConfig {
		std::string title = "Game Window";
		Vector2i windowSize = Vector2i(800, 600);
		bool fullscreen = false;
		bool borderless = false;
		bool resizable = false;
	};

	class Window {
	public:
		Window(const WindowConfig& config = {});
		~Window();

		// No copying
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		// Move only
		Window(Window&& other) noexcept;
		Window& operator=(Window&& other) noexcept;

		// Getters
		Vector2i GetSize() const;
		std::string GetTitle() const;
		bool IsFullscreen() const;
		bool IsBorderless() const;
		bool IsResizable() const;
		bool IsVisible() const;

		// Setters
		void SetTitle(const std::string& title);
		void SetSize(const Vector2i& windowSize);
		void SetFullscreen(bool fullscreen);
		void SetVisible(bool visible);

		// Operations
		void Minimize();
		void Maximize();
		void Restore();
		void Show();
		void Hide();
		void Focus();

		// Mouse input modes
		void SetMouseGrab(bool grab);
		void SetMouseRelativeMode(bool relative);

		// Native handle access (SDL_Window* as void*)
		void* GetNative() const;

		

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};

} // namespace Engine
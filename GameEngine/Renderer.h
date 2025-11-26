#pragma once
#include <memory>
#include "Window.h"
#include "Rect.hpp"
#include "Texture.h"

enum class Flip {
	None,
	Horizontal,
	Vertical,
	Both
};

class Renderer {
public:
	Renderer(Window& window);
	~Renderer();

	void Clear();
	void Present();

	void* GetNative() const;
	void DrawTexture(Texture& tex, const Rect& src, const Rect& dst, Flip flip = Flip::None);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
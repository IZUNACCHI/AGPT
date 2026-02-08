#pragma once
#include <string>
#include "Types.hpp"
#include "Texture.h"

// Simple struct representing a sprite sheet with a texture and frame size
struct SpriteSheet {
	// Name identifier for the sprite sheet
	std::string name;
	// Pointer to the texture containing the sprite sheet
	Texture* texture = nullptr;
	// Size of each frame in the sprite sheet
	Vector2i frameSize = Vector2i::Zero();

	// Checks if the sprite sheet is valid (has a valid texture and positive frame size)
	bool IsValid() const {
		return texture && texture->IsValid()
			&& frameSize.x > 0 && frameSize.y > 0;
	}
};

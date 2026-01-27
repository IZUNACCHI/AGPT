#pragma once

#include "Types.hpp"
#include "EngineException.hpp"
#include "Logger.h"
#include "Surface.h"
#include <memory>
#include <string>

class Renderer;

/// How a texture should be filtered when scaled.
/// Exposed as an engine enum so game code never touches native.
enum class TextureScaleMode {
	Nearest,
	Linear
};

class Texture {
public:
	// Pimpl forward declaration
	struct Impl;

	// Constructor without color key (loads as-is)
	Texture(Renderer& renderer, const std::string& filePath);

	// Constructor Vector3i color key
	Texture(Renderer& renderer, const std::string& filePath, bool useColorKey, const Vector3i& colorKey);

	// Destructor
	~Texture();

	// No copying
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// Move semantics
	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;

	// Getters
	Vector2i GetSize() const;
	void* GetNative() const;
	bool IsValid() const;

	// Filtering
	void SetScaleMode(TextureScaleMode mode);
	TextureScaleMode GetScaleMode() const;

private:
	std::unique_ptr<Impl> impl;
};

#pragma once
#include <memory>
#include <string>

class Renderer;

class Texture {
public:
	Texture(Renderer& renderer, const std::string& path);
	~Texture();

	int Width() const;
	int Height() const;

	void* GetNative() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};



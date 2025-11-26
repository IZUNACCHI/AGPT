#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Texture.h"
#include "Renderer.h"

class AssetManager {
public:
	AssetManager(Renderer& renderer);
	~AssetManager();

	Texture* LoadTexture(const std::string& path);
	void Clear();

private:
	Renderer& renderer;
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
};

#include "AssetManager.h"

AssetManager::AssetManager(Renderer& renderer) : renderer(renderer)
{

}

AssetManager::~AssetManager() {
	Clear();
}

Texture* AssetManager::LoadTexture(const std::string& path) {
	//check if loaded
	auto it = textures.find(path);
	if (it != textures.end())
		return it->second.get();

	// load and store
	auto tex = std::make_unique<Texture>(renderer, path);
	Texture* ptr = tex.get();
	textures[path] = std::move(tex);
	return ptr;
}

void AssetManager::Clear() {
	textures.clear();
}
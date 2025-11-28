#include "AssetManager.h"
#include "Renderer.h"
#include "Texture.h"

struct AssetManager::Impl {
	Renderer* renderer = nullptr;
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures;  // Cache for loaded textures

	~Impl() {
		Clear();
	}

	void Clear() {
		textures.clear();
	}
};

std::unique_ptr<AssetManager::Impl> AssetManager::impl = nullptr;

void AssetManager::Initialize(Renderer& renderer) {
	impl = std::make_unique<Impl>();
	impl->renderer = &renderer;
}

void AssetManager::Shutdown() {
	if (impl) impl->Clear();
	impl.reset();
}

Texture* AssetManager::LoadTexture(const std::string& path) {
	if (!impl || !impl->renderer) return nullptr;

	auto it = impl->textures.find(path);
	if (it != impl->textures.end()) {
		return it->second.get();
	}

	auto tex = std::make_unique<Texture>(*impl->renderer, path);
	Texture* ptr = tex.get();
	impl->textures[path] = std::move(tex);
	return ptr;
}

void AssetManager::Clear() {
	if (impl) impl->Clear();
}
#include "AssetManager.h"
#include "Renderer.h"
#include "Texture.h"
#include "EngineException.h"
#include <iostream>  

// Internal implementation struct
struct AssetManager::Impl {
	Renderer* renderer = nullptr;
	std::string basePath = "";  
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures;  // Cache for loaded textures

	~Impl() {
		Clear();
	}

	void Clear() {
		textures.clear();
	}
};

// Static member initialization
std::unique_ptr<AssetManager::Impl> AssetManager::impl = nullptr;

// Initializes the AssetManager with the given renderer
void AssetManager::Initialize(Renderer& renderer) {
	impl = std::make_unique<Impl>();
	impl->renderer = &renderer;
	if (!impl->renderer) {
		throw EngineException("Renderer is null");
	}
}

// Shuts down the AssetManager and clears all loaded assets
void AssetManager::Shutdown() {
	if (impl) impl->Clear();
	impl.reset();
}

// Sets the base path for asset loading
void AssetManager::SetBasePath(const std::string& base) {
	if (impl) impl->basePath = base;
}

// Loads a texture from the given path, using cache if already loaded
Texture* AssetManager::LoadTexture(const std::string& path) {
	if (!impl || !impl->renderer) {
		std::cerr << "AssetManager not initialized or renderer null when loading: " << path << std::endl;
		return nullptr;
	}

	// Determine full path
	std::string fullPath = path;
	if (!path.empty() && path[0] != '/' && path.find(':') == std::string::npos) {
		fullPath = impl->basePath + path;
	}

	// Use fullPath for caching and loading
	auto it = impl->textures.find(fullPath);
	if (it != impl->textures.end()) {
		return it->second.get();
	}

	// Load new texture
	try {
		auto tex = std::make_unique<Texture>(*impl->renderer, fullPath);
		Texture* ptr = tex.get();
		impl->textures[fullPath] = std::move(tex);
		return ptr;
	}
	catch (const EngineException& e) {
		std::cerr << "Failed to load texture '" << fullPath << "': " << e.what() << std::endl;
		return nullptr;
	}
}

// Clears all loaded assets
void AssetManager::Clear() {
	if (impl) impl->Clear();
}
#pragma once
#include <memory>
#include <string>
#include <unordered_map>

class Renderer;
class Texture;

class AssetManager {
public:
	static void Initialize(Renderer& renderer);
	static void Shutdown();

	static void SetBasePath(const std::string& base);
	static Texture* LoadTexture(const std::string& path);  // returns cached if already loaded
	static void Clear();  // Clears all loaded assets

private:
	struct Impl;
	static std::unique_ptr<Impl> impl;
};
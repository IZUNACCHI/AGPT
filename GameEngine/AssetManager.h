#pragma once

#include "Renderer.h"
#include "Texture.h"
#include "Types.hpp"
#include "Logger.h"
#include <string>
#include <unordered_map>
#include <memory>


class AssetManager {
public:
	AssetManager(Renderer& renderer);

	void SetBasePath(const std::string& basePath);


	Texture* LoadTexture(const std::string& relativePath);
	Texture* LoadTexture(const std::string& relativePath, const Vector3i& colorKey);

	Texture* GetTexture(const std::string& relativePath) const;
	bool IsTextureLoaded(const std::string& relativePath) const;
	void UnloadTexture(const std::string& relativePath);
	void UnloadAllTextures();

private:
	Texture* LoadTextureInternal(const std::string& relativePath, const Vector3i* colorKey);

	Renderer& m_renderer;
	std::string m_basePath;
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
};

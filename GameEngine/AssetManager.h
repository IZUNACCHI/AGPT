#pragma once

#include "Renderer.h"
#include "Texture.h"
#include "SpriteSheet.h"
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

	// --- Sprite sheets ---
	// Sprite sheets are cached assets (Texture + frame size) so multiple Animators can share them.
	SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize);
	SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize);
	SpriteSheet* GetSpriteSheet(const std::string& sheetKey);
	bool IsSpriteSheetLoaded(const std::string& sheetKey) const;
	void UnloadSpriteSheet(const std::string& sheetKey);
	void UnloadAllSpriteSheets();

	Texture* GetTexture(const std::string& relativePath) const;
	bool IsTextureLoaded(const std::string& relativePath) const;
	void UnloadTexture(const std::string& relativePath);
	void UnloadAllTextures();

private:
	Texture* LoadTextureInternal(const std::string& relativePath, const Vector3i* colorKey);
	std::string BuildDefaultSpriteSheetKey(const std::string& textureRelativePath, const Vector2i& frameSize) const;

	Renderer& m_renderer;
	std::string m_basePath;
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
	std::unordered_map<std::string, SpriteSheet> m_spriteSheets;
};
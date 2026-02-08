#pragma once

#include "Renderer.h"
#include "Texture.h"
#include "SpriteSheet.h"
#include "Types.hpp"
#include "Logger.h"
#include "AudioClip.h"
#include <string>
#include <unordered_map>
#include <memory>
#include "BitmapFont.h"


class AssetManager {
public:
	AssetManager(Renderer& renderer);

	void SetBasePath(const std::string& basePath);

	// Default filtering applied to textures when they are loaded.
	void SetDefaultTextureScaleMode(TextureScaleMode mode);
	TextureScaleMode GetDefaultTextureScaleMode() const { return m_defaultTextureScaleMode; }


	Texture* LoadTexture(const std::string& relativePath);
	Texture* LoadTexture(const std::string& relativePath, const Vector3i& colorKey);

	// per-texture override.
	//If the texture is already loaded, this will APPLY the requested mode
	// to the cached texture and return it.
	Texture* LoadTexture(const std::string& relativePath, TextureScaleMode scaleModeOverride);
	Texture* LoadTexture(const std::string& relativePath, const Vector3i& colorKey, TextureScaleMode scaleModeOverride);


	// Sprite sheets are cached assets (Texture + frame size) so multiple Animators can share them.
	SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize);
	SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize);

	// Sprite sheets with a color key 
	SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey);
	SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey);

	// Overloads to apply a scale mode override to the
	// underlying texture used by the sprite sheet.
	SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, TextureScaleMode textureScaleModeOverride);
	SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, TextureScaleMode textureScaleModeOverride);

	// Color key + scale mode override.
	SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride);
	SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride);
	SpriteSheet* GetSpriteSheet(const std::string& sheetKey);
	bool IsSpriteSheetLoaded(const std::string& sheetKey) const;
	void UnloadSpriteSheet(const std::string& sheetKey);
	void UnloadAllSpriteSheets();

	Texture* GetTexture(const std::string& relativePath) const;
	bool IsTextureLoaded(const std::string& relativePath) const;
	void UnloadTexture(const std::string& relativePath);
	void UnloadAllTextures();

	// --- Audio ---
	AudioClip* LoadAudioClip(const std::string& relativePath);
	AudioClip* GetAudioClip(const std::string& relativePath) const;
	bool IsAudioClipLoaded(const std::string& relativePath) const;
	void UnloadAudioClip(const std::string& relativePath);
	void UnloadAllAudioClips();

	// --- Bitmap fonts ---
	BitmapFont* LoadFont(const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar = 32);
	BitmapFont* LoadFont(const std::string& relativePath, const Vector2i& glyphSize, const Vector3i& colorKey, unsigned char firstChar = 32);
	BitmapFont* LoadFont(const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride);

	BitmapFont* LoadFont(const std::string& fontKey, const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar = 32);
	BitmapFont* LoadFont(const std::string& fontKey, const std::string& relativePath, const Vector2i& glyphSize, const Vector3i& colorKey, unsigned char firstChar = 32);
	BitmapFont* LoadFont(const std::string& fontKey, const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride);

	BitmapFont* GetFont(const std::string& keyOrRelativePath) const;
	bool IsFontLoaded(const std::string& keyOrRelativePath) const;
	void UnloadFont(const std::string& keyOrRelativePath);
	void UnloadAllFonts();

private:
	Texture* LoadTextureInternal(const std::string& relativePath, const Vector3i* colorKey, const TextureScaleMode* scaleModeOverride);
	std::string BuildDefaultSpriteSheetKey(const std::string& textureRelativePath, const Vector2i& frameSize) const;

	Renderer& m_renderer;
	std::string m_basePath;
	TextureScaleMode m_defaultTextureScaleMode = TextureScaleMode::Linear;
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
	std::unordered_map<std::string, std::unique_ptr<BitmapFont>> m_fonts;
	std::unordered_map<std::string, SpriteSheet> m_spriteSheets;
	std::unordered_map<std::string, std::unique_ptr<AudioClip>> m_audioClips;
};
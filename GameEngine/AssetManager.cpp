#include "AssetManager.h"
#include "EngineException.hpp"
#include <SDL3/SDL.h>
#include <sstream>
#include <filesystem>



AssetManager::AssetManager(Renderer& renderer)
	: m_renderer(renderer) {
	LOG_INFO("AssetManager initialized");
}

void AssetManager::SetBasePath(const std::string& basePath) {
	m_basePath = basePath;
	if (!m_basePath.empty() && m_basePath.back() != '/' && m_basePath.back() != '\\') {
		m_basePath += '/';
	}
	LOG_INFO("Base path set to: " + m_basePath);
}

void AssetManager::SetDefaultTextureScaleMode(TextureScaleMode mode) {
	m_defaultTextureScaleMode = mode;

	// Apply to already-loaded textures
	for (auto& kv : m_textures) {
		if (kv.second) {
			kv.second->SetScaleMode(mode);
		}
	}
}


Texture* AssetManager::LoadTexture(const std::string& relativePath) {
	return LoadTextureInternal(relativePath, nullptr, nullptr);
}

Texture* AssetManager::LoadTexture(const std::string& relativePath, const Vector3i& colorKey) {
	return LoadTextureInternal(relativePath, &colorKey, nullptr);
}

Texture* AssetManager::LoadTexture(const std::string& relativePath, TextureScaleMode scaleModeOverride) {
	return LoadTextureInternal(relativePath, nullptr, &scaleModeOverride);
}

Texture* AssetManager::LoadTexture(const std::string& relativePath, const Vector3i& colorKey, TextureScaleMode scaleModeOverride) {
	return LoadTextureInternal(relativePath, &colorKey, &scaleModeOverride);
}

// Build default sprite sheet key, Unique per texture + frame size.
std::string AssetManager::BuildDefaultSpriteSheetKey(const std::string& textureRelativePath, const Vector2i& frameSize) const {
	std::stringstream ss;
	ss << textureRelativePath << "|" << frameSize.x << "x" << frameSize.y;
	return ss.str();
}

static std::string BuildDefaultSpriteSheetKeyWithColorKey(const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey) {
	std::stringstream ss;
	ss << textureRelativePath << "|" << frameSize.x << "x" << frameSize.y
		<< "|CK:" << colorKey.x << "," << colorKey.y << "," << colorKey.z;
	return ss.str();
}

static std::string BuildTextureCacheKey(const std::string& relativePath, const Vector3i* colorKey) {
	if (!colorKey) return relativePath;
	std::stringstream ss;
	ss << relativePath << "|CK:" << colorKey->x << "," << colorKey->y << "," << colorKey->z;
	return ss.str();
}

Texture* AssetManager::LoadTextureInternal(const std::string& relativePath, const Vector3i* colorKey, const TextureScaleMode* scaleModeOverride) {
	// Build full path
	std::string fullPath = m_basePath + relativePath;

	const std::string cacheKey = BuildTextureCacheKey(relativePath, colorKey);

	// Check if already loaded
	auto it = m_textures.find(cacheKey);
	if (it != m_textures.end()) {
		LOG_DEBUG("Texture already loaded: " + cacheKey);
		// override: apply to cached texture.
		if (scaleModeOverride && it->second) {
			it->second->SetScaleMode(*scaleModeOverride);
		}
		return it->second.get();
	}

	if (colorKey != nullptr) {
		std::stringstream logMsg;
		logMsg << "Loading texture with color key: " << fullPath
			<< " (R=" << colorKey->x
			<< ", G=" << colorKey->y
			<< ", B=" << colorKey->z << ")";
		LOG_INFO(logMsg.str());
	}
	else {
		LOG_INFO("Loading texture (no color key): " + fullPath);
	}

	try {
		// Create texture based on whether color key is provided
		std::unique_ptr<Texture> texture;

		if (colorKey != nullptr) {
			texture = std::make_unique<Texture>(m_renderer, fullPath, true, *colorKey);
		}
		else {
			texture = std::make_unique<Texture>(m_renderer, fullPath);
		}

		// Apply filtering
		texture->SetScaleMode(scaleModeOverride ? *scaleModeOverride : m_defaultTextureScaleMode);

		auto result = m_textures.emplace(cacheKey, std::move(texture));

		if (!result.second) {
			THROW_ENGINE_EXCEPTION("Failed to cache texture: " + relativePath);
		}

		// Log successful loading
		Texture* loadedTexture = result.first->second.get();
		std::stringstream successMsg;
		successMsg << "Texture loaded: " << relativePath
			<< " (" << loadedTexture->GetSize().x
			<< "x" << loadedTexture->GetSize().y << ")";

		if (colorKey != nullptr) {
			successMsg << " [ColorKey: R=" << colorKey->x
				<< ", G=" << colorKey->y
				<< ", B=" << colorKey->z << "]";
		}

		LOG_INFO(successMsg.str());

		return loadedTexture;
	}
	catch (const EngineException& e) {
		LOG_ERROR("Failed to load texture " + fullPath + ": " + e.what());
		return nullptr;
	}
}

// ---------------- Sprite sheets ----------------

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize) {
	const std::string key = BuildDefaultSpriteSheetKey(textureRelativePath, frameSize);
	return LoadSpriteSheet(key, textureRelativePath, frameSize);
}

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey) {
	const std::string key = BuildDefaultSpriteSheetKeyWithColorKey(textureRelativePath, frameSize, colorKey);
	return LoadSpriteSheet(key, textureRelativePath, frameSize, colorKey);
}

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, TextureScaleMode textureScaleModeOverride) {
	const std::string key = BuildDefaultSpriteSheetKey(textureRelativePath, frameSize);
	return LoadSpriteSheet(key, textureRelativePath, frameSize, textureScaleModeOverride);
}

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride) {
	const std::string key = BuildDefaultSpriteSheetKeyWithColorKey(textureRelativePath, frameSize, colorKey);
	return LoadSpriteSheet(key, textureRelativePath, frameSize, colorKey, textureScaleModeOverride);
}

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize) {
	// Return cached
	auto it = m_spriteSheets.find(sheetKey);
	if (it != m_spriteSheets.end()) {
		return &it->second;
	}

	Texture* texture = LoadTexture(textureRelativePath);
	if (!texture || !texture->IsValid()) {
		LOG_ERROR("Failed to create SpriteSheet '" + sheetKey + "' because texture could not be loaded: " + textureRelativePath);
		return nullptr;
	}

	SpriteSheet sheet;
	sheet.name = sheetKey;
	sheet.texture = texture;
	sheet.frameSize = frameSize;

	auto result = m_spriteSheets.emplace(sheetKey, sheet);
	if (!result.second) {
		return &result.first->second;
	}
	return &result.first->second;
}

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey) {
	// Return cached
	auto it = m_spriteSheets.find(sheetKey);
	if (it != m_spriteSheets.end()) {
		return &it->second;
	}

	Texture* texture = LoadTexture(textureRelativePath, colorKey);
	if (!texture || !texture->IsValid()) {
		LOG_ERROR("Failed to create SpriteSheet '" + sheetKey + "' because texture could not be loaded: " + textureRelativePath);
		return nullptr;
	}

	SpriteSheet sheet;
	sheet.name = sheetKey;
	sheet.texture = texture;
	sheet.frameSize = frameSize;

	auto result = m_spriteSheets.emplace(sheetKey, sheet);
	if (!result.second) {
		return &result.first->second;
	}
	return &result.first->second;
}

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, TextureScaleMode textureScaleModeOverride) {
	// Return cached
	auto it = m_spriteSheets.find(sheetKey);
	if (it != m_spriteSheets.end()) {
		// apply override to cached texture
		if (it->second.texture) {
			it->second.texture->SetScaleMode(textureScaleModeOverride);
		}
		return &it->second;
	}

	Texture* texture = LoadTexture(textureRelativePath, textureScaleModeOverride);
	if (!texture || !texture->IsValid()) {
		LOG_ERROR("Failed to create SpriteSheet '" + sheetKey + "' because texture could not be loaded: " + textureRelativePath);
		return nullptr;
	}

	SpriteSheet sheet;
	sheet.name = sheetKey;
	sheet.texture = texture;
	sheet.frameSize = frameSize;

	auto result = m_spriteSheets.emplace(sheetKey, sheet);
	if (!result.second) {
		return &result.first->second;
	}
	return &result.first->second;
}

SpriteSheet* AssetManager::LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride) {
	// Return cached
	auto it = m_spriteSheets.find(sheetKey);
	if (it != m_spriteSheets.end()) {
		if (it->second.texture) {
			it->second.texture->SetScaleMode(textureScaleModeOverride);
		}
		return &it->second;
	}

	Texture* texture = LoadTexture(textureRelativePath, colorKey, textureScaleModeOverride);
	if (!texture || !texture->IsValid()) {
		LOG_ERROR("Failed to create SpriteSheet '" + sheetKey + "' because texture could not be loaded: " + textureRelativePath);
		return nullptr;
	}

	SpriteSheet sheet;
	sheet.name = sheetKey;
	sheet.texture = texture;
	sheet.frameSize = frameSize;

	auto result = m_spriteSheets.emplace(sheetKey, sheet);
	if (!result.second) {
		return &result.first->second;
	}
	return &result.first->second;
}

SpriteSheet* AssetManager::GetSpriteSheet(const std::string& sheetKey) {
	auto it = m_spriteSheets.find(sheetKey);
	return (it != m_spriteSheets.end()) ? &it->second : nullptr;
}

bool AssetManager::IsSpriteSheetLoaded(const std::string& sheetKey) const {
	return m_spriteSheets.find(sheetKey) != m_spriteSheets.end();
}

void AssetManager::UnloadSpriteSheet(const std::string& sheetKey) {
	auto it = m_spriteSheets.find(sheetKey);
	if (it != m_spriteSheets.end()) {
		m_spriteSheets.erase(it);
	} else {
		LOG_WARN("SpriteSheet not found for unloading: " + sheetKey);
	}
}

void AssetManager::UnloadAllSpriteSheets() {
	std::stringstream logMsg;
	logMsg << "Unloading all sprite sheets (count: " << m_spriteSheets.size() << ")";
	LOG_INFO(logMsg.str());
	m_spriteSheets.clear();
}

Texture* AssetManager::GetTexture(const std::string& relativePath) const {
	auto it = m_textures.find(relativePath);
	if (it != m_textures.end()) {
		return it->second.get();
	}
	return nullptr;
}

bool AssetManager::IsTextureLoaded(const std::string& relativePath) const {
	return m_textures.find(relativePath) != m_textures.end();
}

void AssetManager::UnloadTexture(const std::string& relativePath) {
	auto it = m_textures.find(relativePath);
	if (it != m_textures.end()) {
		Texture* doomed = it->second.get();
		LOG_INFO("Unloading texture: " + relativePath);
		m_textures.erase(it);

		// Remove any sprite sheets that referenced this texture.
		for (auto sheetIt = m_spriteSheets.begin(); sheetIt != m_spriteSheets.end(); ) {
			if (sheetIt->second.texture == doomed) {
				sheetIt = m_spriteSheets.erase(sheetIt);
			} else {
				++sheetIt;
			}
		}

		// Remove any fonts that referenced this texture.
		for (auto fontIt = m_fonts.begin(); fontIt != m_fonts.end(); ) {
			if (fontIt->second && fontIt->second->GetTexture() == doomed) {
				fontIt = m_fonts.erase(fontIt);
			}
			else {
				++fontIt;
			}
		}
	}
	else {
		LOG_WARN("Texture not found for unloading: " + relativePath);
	}
}

void AssetManager::UnloadAllTextures() {
	std::stringstream logMsg;
	logMsg << "Unloading all textures (count: " << m_textures.size() << ")";
	LOG_INFO(logMsg.str());
	m_textures.clear();

	// All cached sprite sheets become invalid when textures are gone.
	UnloadAllSpriteSheets();
	UnloadAllFonts();
	UnloadAllAudioClips();
}

// ---------------- Audio clips ----------------

AudioClip* AssetManager::LoadAudioClip(const std::string& relativePath) {
	// Check if already loaded
	auto it = m_audioClips.find(relativePath);
	if (it != m_audioClips.end()) {
		return it->second.get();
	}

	const std::string fullPath = m_basePath + relativePath;
	SDL_AudioSpec spec{};
	SDL_zero(spec);
	Uint8* audioBuf = nullptr;
	Uint32 audioLen = 0;
	// Load WAV file
	if (!SDL_LoadWAV(fullPath.c_str(), &spec, &audioBuf, &audioLen)) {
		THROW_ENGINE_EXCEPTION("Failed to load WAV '" + relativePath + "': " + std::string(SDL_GetError()));
	}
	// Create AudioClip
	auto clip = std::make_unique<AudioClip>();
	clip->name = relativePath;
	clip->spec = spec;
	clip->pcm.assign(audioBuf, audioBuf + audioLen);
	SDL_free(audioBuf);

	// Cache and return
	AudioClip* result = clip.get();
	m_audioClips.emplace(relativePath, std::move(clip));
	LOG_INFO("Loaded AudioClip: " + relativePath);
	return result;
}

AudioClip* AssetManager::GetAudioClip(const std::string& relativePath) const {
	auto it = m_audioClips.find(relativePath);
	if (it == m_audioClips.end()) return nullptr;
	return it->second.get();
}

bool AssetManager::IsAudioClipLoaded(const std::string& relativePath) const {
	return m_audioClips.find(relativePath) != m_audioClips.end();
}

void AssetManager::UnloadAudioClip(const std::string& relativePath) {
	auto it = m_audioClips.find(relativePath);
	if (it != m_audioClips.end()) {
		m_audioClips.erase(it);
		LOG_INFO("Unloaded AudioClip: " + relativePath);
	}
}

void AssetManager::UnloadAllAudioClips() {
	std::stringstream logMsg;
	logMsg << "Unloading all audio clips (count: " << m_audioClips.size() << ")";
	LOG_INFO(logMsg.str());
	m_audioClips.clear();
}

// ---------------- Bitmap fonts ----------------

BitmapFont* AssetManager::LoadFont(const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar)
{
	return LoadFont(relativePath, relativePath, glyphSize, firstChar);
}

BitmapFont* AssetManager::LoadFont(const std::string& fontKey, const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar)
{
	// Return cached
	auto it = m_fonts.find(fontKey);
	if (it != m_fonts.end()) {
		return it->second.get();
	}

	// Load texture
	Texture* texture = LoadTexture(relativePath);
	if (!texture || !texture->IsValid()) {
		LOG_ERROR("Failed to create Font '" + fontKey + "' because texture could not be loaded: " + relativePath);
		return nullptr;
	}

	// Create font, cache abd return
	auto font = std::make_unique<BitmapFont>(texture, glyphSize, firstChar);
	BitmapFont* out = font.get();
	m_fonts.emplace(fontKey, std::move(font));
	return out;
}

BitmapFont* AssetManager::LoadFont(const std::string& relativePath, const Vector2i& glyphSize, const Vector3i& colorKey, unsigned char firstChar)
{
	return LoadFont(relativePath, relativePath, glyphSize, colorKey, firstChar);
}

BitmapFont* AssetManager::LoadFont(const std::string& fontKey, const std::string& relativePath, const Vector2i& glyphSize, const Vector3i& colorKey, unsigned char firstChar)
{
	// Use default texture filtering for the scene
	return LoadFont(fontKey, relativePath, glyphSize, firstChar, colorKey, m_defaultTextureScaleMode);
}


BitmapFont* AssetManager::LoadFont(const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride)
{
	return LoadFont(relativePath, relativePath, glyphSize, firstChar, colorKey, textureScaleModeOverride);
}

BitmapFont* AssetManager::LoadFont(const std::string& fontKey, const std::string& relativePath, const Vector2i& glyphSize, unsigned char firstChar, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride)
{
	auto it = m_fonts.find(fontKey);
	if (it != m_fonts.end()) {
		return it->second.get();
	}

	Texture* texture = LoadTexture(relativePath, colorKey, textureScaleModeOverride);
	if (!texture || !texture->IsValid()) {
		LOG_ERROR("Failed to create Font '" + fontKey + "' because texture could not be loaded: " + relativePath);
		return nullptr;
	}

	auto font = std::make_unique<BitmapFont>(texture, glyphSize, firstChar);
	BitmapFont* out = font.get();
	m_fonts.emplace(fontKey, std::move(font));
	return out;
}

BitmapFont* AssetManager::GetFont(const std::string& keyOrRelativePath) const
{
	auto it = m_fonts.find(keyOrRelativePath);
	if (it != m_fonts.end()) {
		return it->second.get();
	}
	return nullptr;
}

bool AssetManager::IsFontLoaded(const std::string& keyOrRelativePath) const
{
	return m_fonts.find(keyOrRelativePath) != m_fonts.end();
}

void AssetManager::UnloadFont(const std::string& keyOrRelativePath)
{
	auto it = m_fonts.find(keyOrRelativePath);
	if (it != m_fonts.end()) {
		LOG_INFO("Unloading font: " + keyOrRelativePath);
		m_fonts.erase(it);
	}
	else {
		LOG_WARN("Font not found for unloading: " + keyOrRelativePath);
	}
}

void AssetManager::UnloadAllFonts()
{
	std::stringstream logMsg;
	logMsg << "Unloading all Fonts (count: " << m_fonts.size() << ")";
	LOG_INFO(logMsg.str());
	m_fonts.clear();
}

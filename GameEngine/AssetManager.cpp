#include "AssetManager.h"
#include "EngineException.hpp"
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

Texture* AssetManager::LoadTexture(const std::string& relativePath) {
	return LoadTextureInternal(relativePath, nullptr);
}

Texture* AssetManager::LoadTexture(const std::string& relativePath, const Vector3i& colorKey) {
	return LoadTextureInternal(relativePath, &colorKey);
}

std::string AssetManager::BuildDefaultSpriteSheetKey(const std::string& textureRelativePath, const Vector2i& frameSize) const {
	// Unique per texture + frame size.
	std::stringstream ss;
	ss << textureRelativePath << "|" << frameSize.x << "x" << frameSize.y;
	return ss.str();
}

Texture* AssetManager::LoadTextureInternal(const std::string& relativePath, const Vector3i* colorKey) {
	// Build full path
	std::string fullPath = m_basePath + relativePath;

	// Check if already loaded
	auto it = m_textures.find(relativePath);
	if (it != m_textures.end()) {
		LOG_DEBUG("Texture already loaded: " + relativePath);
		return it->second.get();
	}

	// Log appropriate message based on whether we're using a color key
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

		auto result = m_textures.emplace(relativePath, std::move(texture));

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

	// Insert without C++17 structured bindings
	auto result = m_spriteSheets.emplace(sheetKey, sheet);
	if (!result.second) {
		// Extremely unlikely because we checked above, but keep it safe.
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
}
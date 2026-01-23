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
		LOG_INFO("Unloading texture: " + relativePath);
		m_textures.erase(it);
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
}
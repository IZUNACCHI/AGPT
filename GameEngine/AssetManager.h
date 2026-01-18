#pragma once

#include "Renderer.h"
#include "Texture.h"
#include "Types.hpp"
#include "Logger.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace Engine {

	class AssetManager {
	public:
		AssetManager(Renderer& renderer);

		void SetBasePath(const std::string& basePath);

		
		Texture* LoadTexture(const std::string& relativePath, Vector3i* colorKey = nullptr);

		Texture* GetTexture(const std::string& relativePath) const;
		bool IsTextureLoaded(const std::string& relativePath) const;
		void UnloadTexture(const std::string& relativePath);
		void UnloadAllTextures();

	private:
		Renderer& m_renderer;
		std::string m_basePath;
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
	};

} // namespace Engine
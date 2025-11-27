#pragma once
#include "Scene.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>

class SceneManager
{
public:
	// Call once at engine startup (SleeplessEngine will do this automatically)
	static void Initialize();

	// Register a scene type so it can be loaded by name (optional but recommended for level lists/editors)
	template<typename T>
	static void RegisterScene(const std::string& sceneName)
	{
		static_assert(std::is_base_of_v<Scene, T>, "T must inherit from Scene");
		scenes[sceneName] = []() -> std::unique_ptr<Scene> { return std::make_unique<T>(); };
	}

	// Load by name (string) - perfect for level select screens, save/load, etc.
	static void LoadScene(const std::string& sceneName);

	// Load by type (type-safe, no registration needed) - fastest for normal gameplay
	template<typename T = Scene>
	static void LoadScene()
	{
		static_assert(std::is_base_of_v<Scene, T>, "T must inherit from Scene");

		if (currentScene)
			currentScene->OnUnload();

		currentScene = std::make_unique<T>();
		currentScene->name = typeid(T).name(); // optional debug name
		currentScene->OnLoad();
	}

	// Get the currently active scene (never nullptr after first LoadScene)
	static Scene* GetActiveScene() { return currentScene.get(); }

private:
	static std::unique_ptr<Scene> currentScene;
	static std::unordered_map<std::string, std::function<std::unique_ptr<Scene>()>> scenes;
};


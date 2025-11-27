#include "SceneManager.h"

// Pointer to the current active scene
std::unique_ptr<Scene> SceneManager::currentScene = nullptr;
// Map of registered scenes
std::unordered_map<std::string, std::function<std::unique_ptr<Scene>()>> SceneManager::scenes;

// Initialize the SceneManager with a default empty scene
void SceneManager::Initialize()
{
	// Create and set an initial empty scene
	currentScene = std::make_unique<Scene>();
	currentScene->name = "EmptyDefaultScene";
}

// Load a scene by its registered name
void SceneManager::LoadScene(const std::string& sceneName)
{	
	// Find the scene in the registered scenes map
	auto it = scenes.find(sceneName);
	// If not found, do nothing
	if (it == scenes.end()) return;

	// Unload the current scene if it exists
	if (currentScene) currentScene->OnUnload();

	// Create the new scene using the registered factory function
	currentScene = it->second();
	currentScene->name = sceneName;
	currentScene->OnLoad();
}
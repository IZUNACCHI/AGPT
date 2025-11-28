#include "SceneManager.h"

// Pointer to the current active scene
std::unique_ptr<Scene> SceneManager::currentScene = nullptr;

// Initialize the SceneManager (no default scene creation)
void SceneManager::Initialize()
{
	// No default scene; user must explicitly set the first scene
}

// Set a new active scene, unloading the current one if present
void SceneManager::SetActiveScene(std::unique_ptr<Scene> newScene)
{
	if (currentScene)
	{
		currentScene->OnUnload();
	}

	currentScene = std::move(newScene);
	if (currentScene)
	{
		currentScene->OnLoad();
	}
}
#pragma once
#include "Scene.h"
#include <memory>
#include <string>

class SceneManager
{
public:
	// Call once at engine startup (SleeplessEngine will do this automatically)
	static void Initialize();

	// Set a new active scene, swapping out the current one if it exists
	// Takes ownership of the provided scene pointer
	static void SetActiveScene(std::unique_ptr<Scene> newScene);

	// Get the currently active scene (may be nullptr if none set)
	static Scene* GetActiveScene() { return currentScene.get(); }

	// Templated getter for typed access
	template<typename T = Scene>
	static T* GetActiveScene()
	{
		static_assert(std::is_base_of_v<Scene, T>, "T must inherit from Scene");
		return static_cast<T*>(currentScene.get());
	}

private:
	static std::unique_ptr<Scene> currentScene;
};
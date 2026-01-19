#pragma once

#include "Scene.h"
#include "Window.h"
#include "Renderer.h"
#include "AssetManager.h"
#include <memory>
#include "Time.hpp"
#include <string>
#include "Input.h"


class Scene;
class Window;
class Renderer;
class AssetManager;

struct Config {
		float fixedDeltaTime = 1.0f / 60.0f;
		float maximumDeltaTime = 0.25f;
		float targetFPS = 60.0f;

		// Asset base path
		std::string assetBasePath = "Dist/graphics/";
		WindowConfig windowConfig{};
};

class SleeplessEngine {
public:
	static SleeplessEngine& GetInstance();

	void Initialize(Config config);
	void Run();
	void Shutdown();

	// Changed to raw pointer since user manages scene lifecycle
	void SetScene(Scene* scene);

private:
	SleeplessEngine() = default;

	// Core loop phases
	void Update(float deltaTime);
	void FixedUpdate(float fixedDeltaTime);
	void LateUpdate(float deltaTime);
	void Render();
	void GarbageCollect();

private:
	bool m_isInitialized = false;
	bool m_isRunning = false;

	Config m_config{};

	// --- Time system (defined here) ---
	Time& m_time = Time::Instance();

	// --- Engine systems ---
	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<AssetManager> m_assetManager;

	// Changed to raw pointer
	Scene* m_currentScene = nullptr;
};
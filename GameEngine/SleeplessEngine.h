#pragma once

#include "Scene.h"
#include "Window.h"
#include "Renderer.h"
#include "AssetManager.h"
#include <memory>
#include <string>
#include "Physics2D.h"

#include "SpriteRenderer.h"




struct Config {
		float fixedDeltaTime = 1.0f / 60.0f;
		float maximumDeltaTime = 0.25f;
		float targetFPS = 60.0f;

		// Asset base path
		std::string assetBasePath = "Dist/graphics/";
		WindowConfig windowConfig{};

		// Sprite sorting options
		SpriteRenderer::SortOptions spriteSortOptions{
			SpriteRenderer::SortAxis::Y,
			SpriteRenderer::SortAxis::X,
			true,
			true
		};

		bool debugDrawColliders = true;
};

class SleeplessEngine {
public:
	static SleeplessEngine& GetInstance();

	void Initialize(Config config);
	void Run();
	void Shutdown();

	// Changed to raw pointer since user manages scene lifecycle
	void SetScene(Scene* scene);
	void ResetPhysicsWorld(const Vector2f& gravity = Vector2f(0.0f, -9.81f));
	Physics2DWorld* GetPhysicsWorld() const { return m_physicsWorld.get(); }

	// Expose renderer
	Renderer* GetRenderer() const { return m_renderer.get(); }
	//Expse window 
	Window* GetWindow() const { return m_window.get(); }

	AssetManager* GetAssetManager() const { return m_assetManager.get(); }

private:
	SleeplessEngine() = default;

	// Core loop phases
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Render();
	void DestroyPending();

private:
	bool m_isInitialized = false;
	bool m_isRunning = false;

	Config m_config{};

	// --- Engine systems ---
	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<AssetManager> m_assetManager;
	std::unique_ptr<Physics2DWorld> m_physicsWorld;

	// Changed to raw pointer
	Scene* m_currentScene = nullptr;
};
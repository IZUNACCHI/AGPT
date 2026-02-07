#pragma once

#include "Scene.h"
#include "Window.h"
#include "Renderer.h"
#include "AssetManager.h"
#include <memory>
#include <string>
#include "Physics2D.h"
#include "GameInstance.h"
#include <functional>
#include <type_traits>

#include "SpriteRenderer.h"




struct Config {
	float fixedDeltaTime = 1.0f / 60.0f;
	float maximumDeltaTime = 0.25f;
	float targetFPS = 144.f;

	// Asset base path
	std::string assetBasePath = "Dist/graphics/";
	WindowConfig windowConfig{};

	// Virtual Resolution / Letterboxed Viewport
	Vector2i virtualResolution = Vector2i(0, 0);

	ViewportScaleMode viewportScaleMode = ViewportScaleMode::Letterbox;
	bool integerScale = false;

	// If enabled (windowed mode only), the engine will "snap" the window size
	// to the closest size that matches the virtual aspect ratio, using the
	// smallest dimension as the limiter. This avoids bars by resizing the window.
	// (Only meaningful with Letterbox scale mode.)
	bool fitWindowToScale = false;

	// Colors (RGBA 0-255)
	Vector4i clearColor = Vector4i(0, 0, 0, 255);
	Vector4i letterboxColor = Vector4i(0, 0, 0, 255);

	// Default texture filtering when scaling
	TextureScaleMode textureScaleMode = TextureScaleMode::Linear;

	// Sprite sorting options
	SpriteRenderer::SortOptions spriteSortOptions{
		SpriteRenderer::SortAxis::Y,
		SpriteRenderer::SortAxis::X,
		true,
		true
	};

	bool debugDrawColliders = false;
};

class SleeplessEngine {
public:
	static SleeplessEngine& GetInstance();

	void Initialize(Config config);
	void Run();
	void Shutdown();

	/// Sets the factory used to create the GameInstance at engine startup.
	/// If not set, the engine creates a default empty GameInstance.
	void SetGameInstanceFactory(std::function<std::unique_ptr<GameInstance>()> factory);

	/// helper: just provide the type. The type must derive from GameInstance and be default-constructible.
	template<typename TInstance>
	void SetGameInstanceType() {
		static_assert(std::is_base_of<GameInstance, TInstance>::value, "TInstance must derive from GameInstance");
		SetGameInstanceFactory([]() { return std::make_unique<TInstance>(); });
	}
	GameInstance* GetGameInstance() const { return m_gameInstance.get(); }

	template<typename T>
	T* GetGameInstanceAs() const { return dynamic_cast<T*>(m_gameInstance.get()); }

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

	void CreateGameInstanceIfNeeded();

	void FixedUpdate();
	void LateUpdate();
	void Render();
	void DestroyPending();
	void ApplyFitWindowToScale();

private:
	bool m_isInitialized = false;
	bool m_isRunning = false;

	Config m_config{};

	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<AssetManager> m_assetManager;
	std::unique_ptr<Physics2DWorld> m_physicsWorld;

	Scene* m_currentScene = nullptr;
	Vector2i m_lastFitWindowSize = Vector2i(0, 0);

	// Game instance
	// Created once at engine startup and kept alive until Shutdown()
	std::unique_ptr<GameInstance> m_gameInstance;
	// Factory set by the game layer to create a derived GameInstance
	std::function<std::unique_ptr<GameInstance>()> m_gameInstanceFactory;
	void RequestQuit();
};
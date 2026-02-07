#include "SleeplessEngine.h"
#include "Object.h"
#include "Logger.h"
#include "Time.hpp"
#include "Input.h"
#include "GameInstance.h"
#include "TextRenderer.h"
#include "RenderQueue.h"
#include "RenderSystem.h"
#include "UISystem.h"
#include "Audio.h"
#include <SDL3/SDL.h>
#include <cmath>

namespace {
	RenderQueue::SortAxis ToQueueAxis(SpriteRenderer::SortAxis a) {
		switch (a) {
		case SpriteRenderer::SortAxis::X: return RenderQueue::SortAxis::X;
		case SpriteRenderer::SortAxis::Y: return RenderQueue::SortAxis::Y;
		case SpriteRenderer::SortAxis::None:
		default: return RenderQueue::SortAxis::None;
		}
	}

	RenderQueue::SortOptions ToQueueSortOptions(const SpriteRenderer::SortOptions& o) {
		RenderQueue::SortOptions q;
		q.primaryAxis = ToQueueAxis(o.primaryAxis);
		q.secondaryAxis = ToQueueAxis(o.secondaryAxis);
		q.primaryAscending = o.primaryAscending;
		q.secondaryAscending = o.secondaryAscending;
		return q;
	}
}

SleeplessEngine& SleeplessEngine::GetInstance() {
	static SleeplessEngine instance;
	return instance;
}


void SleeplessEngine::SetGameInstanceFactory(std::function<std::unique_ptr<GameInstance>()> factory) {
	m_gameInstanceFactory = std::move(factory);
}

void SleeplessEngine::CreateGameInstanceIfNeeded() {
	if (m_gameInstance) {
		return;
	}
	if (m_gameInstanceFactory) {
		m_gameInstance = m_gameInstanceFactory();
	}
	if (!m_gameInstance) {
		m_gameInstance = std::make_unique<GameInstance>();
	}
	m_gameInstance->OnInit();
}

void SleeplessEngine::Initialize(Config config) {
	if (m_isInitialized) {
		return;
	}

	Logger::getInstance().init(true);
	m_config = config;

	// --- Time ---
	Time::Initialize();
	Time::SetFixedDeltaTime(m_config.fixedDeltaTime);
	Time::SetMaxDeltaTime(m_config.maximumDeltaTime);
	Time::SetTargetFPS(m_config.targetFPS);


	try {
		// --- SDL ---
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
			THROW_ENGINE_EXCEPTION("Failed to initialize SDL: " + std::string(SDL_GetError()));
		}

		// --- Audio --- (init SDL audio subsystem + open default device)
		Audio::Initialize();

		// --- Window / Renderer / Assets / Input ---
		m_window = std::make_unique<Window>(config.windowConfig);
		m_window->SetVisible(true);
		m_renderer = std::make_unique<Renderer>(*m_window);
		// Virtual resolution / viewport scaling (optional)
		m_renderer->SetVirtualResolution(m_config.virtualResolution);
		ViewportScaleMode mode = m_config.viewportScaleMode;
		m_renderer->SetViewportScaleMode(mode);
		m_renderer->SetIntegerScaling(m_config.integerScale);
		m_renderer->SetClearColor(m_config.clearColor);
		m_renderer->SetLetterboxColor(m_config.letterboxColor);
		SpriteRenderer::SetSortOptions(m_config.spriteSortOptions);
		RenderQueue::SetSpriteSortOptions(ToQueueSortOptions(m_config.spriteSortOptions));
		m_assetManager = std::make_unique<AssetManager>(*m_renderer);
		m_assetManager->SetBasePath(m_config.assetBasePath);
		m_assetManager->SetDefaultTextureScaleMode(m_config.textureScaleMode);
		Input::Initialize();

		m_physicsWorld = std::make_unique<Physics2DWorld>();
		m_physicsWorld->Initialize(Vector2(0, 0));

		// GameInstance is created once per engine lifetime.
		CreateGameInstanceIfNeeded();

		m_isInitialized = true;
	}
	catch (const std::exception& e) {
		std::cerr << "Engine initialization failed. Shutting down." << e.what() << std::endl;
		Shutdown();
	}
}

void SleeplessEngine::Run() {
	try {
		if (!m_isInitialized || !m_currentScene) {
			THROW_ENGINE_EXCEPTION("Engine not initialized or no scene set");
		}

		m_isRunning = true;

		while (m_isRunning) {
			// 1. Time
			Time::Tick();

			// 2. Input
			Input::PollEvents();
			ApplyFitWindowToScale();
			if (Input::ShouldQuit()) {
				RequestQuit();
				break;
			}
			if (Input::IsKeyPressed(Key::F9)) {
				Time::ToggleShowFPS();
			}

			// 3. Fixed update
			int steps = Time::CalculateFixedSteps();
			for (int i = 0; i < steps; ++i) {
				FixedUpdate();
				Time::ConsumeFixedStep();
			}

			// 4. Variable update
			Update();
			if (!m_isRunning) {
				break;
			}

			// 5. Late update
			LateUpdate();
			if (!m_isRunning) {
				break;
			}

			// 6. Garbage collection
			DestroyPending();

			// 7. Render
			Render();

			Time::WaitForTargetFPS();
		}

		// Teardown happens once, after the loop stops.
		Shutdown();
	}
	catch (const std::exception& e) {
		std::cerr << "Something" << e.what() << std::endl;
		RequestQuit();
		Shutdown();
	}
}

void SleeplessEngine::SetScene(Scene* scene) {
	if (m_currentScene) {
		m_currentScene->Unload();
	}

	m_currentScene = scene;

	if (m_currentScene) {
		m_currentScene->Start();
	}
}

void SleeplessEngine::ResetPhysicsWorld(const Vector2f& gravity) {
	if (!m_physicsWorld) {
		m_physicsWorld = std::make_unique<Physics2DWorld>();
	}
	m_physicsWorld->Reset(gravity);
}

void SleeplessEngine::ApplyFitWindowToScale() {
	if (!m_config.fitWindowToScale) return;
	if (!m_window || !m_renderer) return;
	if (m_window->IsFullscreen()) return; // don't fight fullscreen

	const Vector2i virt = m_config.virtualResolution;
	if (virt.x <= 0 || virt.y <= 0) return;
	if (m_renderer->GetViewportScaleMode() != ViewportScaleMode::Letterbox) return;

	const Vector2i cur = m_window->GetSize();
	if (cur.x <= 0 || cur.y <= 0) return;

	const float targetAspect = (float)virt.x / (float)virt.y;
	const float curAspect = (float)cur.x / (float)cur.y;

	// Already close enough (avoid jitter).
	if (std::abs(curAspect - targetAspect) < 0.0005f) {
		return;
	}

	int newW = cur.x;
	int newH = cur.y;

	// Snap by reducing the "excess" dimension, keeping the limiting dimension.
	if (curAspect > targetAspect) {
		// Window is too wide: reduce width.
		newW = (int)std::round((float)cur.y * targetAspect);
		newH = cur.y;
	}
	else {
		// Window is too tall: reduce height.
		newW = cur.x;
		newH = (int)std::round((float)cur.x / targetAspect);
	}

	newW = std::max(1, newW);
	newH = std::max(1, newH);

	// If integer scaling is enabled, optionally snap to the largest integer
	// multiple that fits inside the chosen size.
	if (m_config.integerScale) {
		const int sX = newW / virt.x;
		const int sY = newH / virt.y;
		const int s = std::min(sX, sY);
		if (s >= 1) {
			newW = virt.x * s;
			newH = virt.y * s;
		}
	}

	// Avoid spamming SetSize every frame.
	if (std::abs(newW - cur.x) <= 1 && std::abs(newH - cur.y) <= 1) return;
	Vector2i desired(newW, newH);
	if (desired.x == m_lastFitWindowSize.x && desired.y == m_lastFitWindowSize.y) return;

	m_lastFitWindowSize = desired;
	m_window->SetSize(desired);
}


void SleeplessEngine::RequestQuit()
{
	m_isRunning = false;
}

inline void SleeplessEngine::Update() {
	if (!m_isRunning) return;
	if (m_currentScene && m_currentScene->IsActive()) {
		m_currentScene->Update();
	}
	// Scene update may have requested quit.
	if (!m_isRunning) return;

	// UI runs after gameplay update so it can consume input consistently.
	UISystem::Get().Update();
}

void SleeplessEngine::FixedUpdate() {
	if (m_currentScene && m_currentScene->IsActive()) {
		m_currentScene->FixedUpdate();
	}

	if (m_physicsWorld) {
		m_physicsWorld->Step(Time::FixedDeltaTime(), 20);
	}
}

void SleeplessEngine::LateUpdate() {
	if (m_currentScene && m_currentScene->IsActive()) {
		m_currentScene->LateUpdate();
	}
}

void SleeplessEngine::Render() {
	m_renderer->Clear();

	if (m_currentScene && m_currentScene->IsActive()) {
		RenderQueue queue;
		RenderSystem::Get().BuildQueue(queue);
		queue.Execute(*m_renderer);
		m_currentScene->Render();
	}

	if (m_config.debugDrawColliders && m_physicsWorld) {
		m_physicsWorld->DebugDraw(*m_renderer);
	}

	// UI always renders last so it overlays the world/debug.
	UISystem::Get().Render(*m_renderer);

	m_renderer->Present();
}

void SleeplessEngine::DestroyPending() {
	Object::ProcessDestroyQueue();
}

void SleeplessEngine::Shutdown() {
	// If Shutdown() is called while the main loop is still running (e.g. from gameplay/UI),
	// defer the teardown until after the loop exits.
	if (m_isRunning) {
		RequestQuit();
		return;
	}

	m_isRunning = false;

	if (m_currentScene) {
		m_currentScene->Unload();
	}

	if (m_gameInstance) {
		m_gameInstance->OnShutdown();
		m_gameInstance.reset();
	}

	// UI may query input; clear it first.
	UISystem::Get().Shutdown();

	Input::Shutdown();
	Audio::Shutdown();
	if (m_physicsWorld) {
		m_physicsWorld->Shutdown();
		m_physicsWorld.reset();
	}
	m_assetManager.reset();
	m_renderer.reset();
	m_window.reset();
	SDL_Quit();

	m_isInitialized = false;
}
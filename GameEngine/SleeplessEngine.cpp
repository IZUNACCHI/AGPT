#include "SleeplessEngine.h"
#include "Object.h"
#include <SDL3/SDL.h>

SleeplessEngine& SleeplessEngine::GetInstance() {
	static SleeplessEngine instance;
	return instance;
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

		// --- Window / Renderer / Assets / Input ---
		m_window = std::make_unique<Window>(config.windowConfig);
		m_window->SetVisible(true);
		m_renderer = std::make_unique<Renderer>(*m_window);
		m_assetManager = std::make_unique<AssetManager>(*m_renderer);
		Input::Initialize();

		m_physicsWorld = std::make_unique<Physics2DWorld>();
		m_physicsWorld->Initialize();

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
			if (Input::ShouldQuit()) {
				Shutdown();
				return;
			}
			if (Input::IsKeyPressed(Key::F9)) {
				Time::ToggleShowFPS();
			}

			// 3. Fixed update
			int steps = Time::CalculateFixedSteps();
			for (int i = 0; i < steps; ++i) {
				FixedUpdate(Time::FixedDeltaTime());
				Time::ConsumeFixedStep();
			}

			// 4. Variable update
			Update(Time::DeltaTime());

			// 5. Late update
			LateUpdate(Time::DeltaTime());

			// 6. Garbage collection
			DestroyPending();

			// 7. Render
			Render();

			Time::WaitForTargetFPS();
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Something" << e.what() << std::endl;
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

inline void SleeplessEngine::Update(float deltaTime) {
	if (m_currentScene && m_currentScene->IsActive()) {
		m_currentScene->Update(deltaTime);
	}
}

void SleeplessEngine::FixedUpdate(float fixedDeltaTime) {
	if (m_currentScene && m_currentScene->IsActive()) {
		m_currentScene->FixedUpdate(fixedDeltaTime);
	}

	if (m_physicsWorld) {
		m_physicsWorld->Step(fixedDeltaTime, 1);
	}
}

void SleeplessEngine::LateUpdate(float deltaTime) {
	if (m_currentScene && m_currentScene->IsActive()) {
		m_currentScene->LateUpdate(deltaTime);
	}
}

void SleeplessEngine::Render() {
	m_renderer->Clear();

	if (m_currentScene && m_currentScene->IsActive()) {
		m_currentScene->Render();
	}

	m_renderer->Present();
}

void SleeplessEngine::DestroyPending() {
	Object::ProcessDestroyQueue();
}

void SleeplessEngine::Shutdown() {
	m_isRunning = false;

	if (m_currentScene) {
		m_currentScene->Unload();
	}

	Input::Shutdown();
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

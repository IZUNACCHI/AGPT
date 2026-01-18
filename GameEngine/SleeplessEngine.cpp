#include "SleeplessEngine.h"
#include <SDL3/SDL.h>

// Engine subsystems

namespace Engine {

	SleeplessEngine& SleeplessEngine::GetInstance() {
		static SleeplessEngine instance;
		return instance;
	}

	void SleeplessEngine::Initialize(Config config) {
		if (m_isInitialized)
			return;
		
			Logger::getInstance().init(true);
			m_config = std::move(config);

			// --- Time ---
			m_time.Initialize();
			m_time.SetFixedDeltaTime(m_config.fixedDeltaTime);
			m_time.SetMaxDeltaTime(m_config.maximumDeltaTime);
			m_time.SetTargetFPS(m_config.targetFPS);
			try
			{
				SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
			// --- SDL ---
			if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
				THROW_ENGINE_EXCEPTION("Failed to initialize SDL: 1 ") << SDL_GetError();
			}

			// --- Window / Renderer / Assets ---
			m_window = std::make_unique<Window>(config.windowConfig);
			m_window->SetVisible(true);
			m_renderer = std::make_unique<Renderer>(*m_window);
			m_assetManager = std::make_unique<AssetManager>(*m_renderer);

			// Removed initial scene loading from config
			// User will call SetScene manually

			m_isInitialized = true;
			}
			catch (const std::exception& e)
			{
				std::cerr << "Engine initialization failed. Shutting down." << e.what() << std::endl;
				Shutdown();
			}
		
	}

	void SleeplessEngine::Run() {
		try
		{
			if (!m_isInitialized || !m_currentScene)  // Still need a scene to run
				THROW_ENGINE_EXCEPTION("Engine not initialized or no scene set");

			m_isRunning = true;

			while (m_isRunning) {
				// 1. Time
				m_time.Tick();

				// 2. Input
				// Input::Update();

				// 3. Fixed update
				int steps = m_time.CalculateFixedSteps();
				for (int i = 0; i < steps; ++i) {
					FixedUpdate(m_time.FixedDeltaTime());
					m_time.ConsumeFixedStep();
				}

				// 4. Variable update
				Update(m_time.DeltaTime());

				// 5. Late update
				LateUpdate(m_time.DeltaTime());

				// 6. Render
				Render();

				// 7. Cleanup
				GarbageCollect();
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Something" << e.what() << std::endl;
			Shutdown();
		}
		
	}

	void SleeplessEngine::SetScene(Scene* scene) {
		if (m_currentScene)
			m_currentScene->Unload();

		m_currentScene = scene;

		if (m_currentScene)
			m_currentScene->Start();
	}

	void SleeplessEngine::Update(float deltaTime) {
		if (m_currentScene && m_currentScene->IsActive())
			m_currentScene->Update(deltaTime);
	}

	void SleeplessEngine::FixedUpdate(float fixedDeltaTime) {
		if (m_currentScene && m_currentScene->IsActive())
			m_currentScene->FixedUpdate(fixedDeltaTime);
	}

	void SleeplessEngine::LateUpdate(float deltaTime) {
		if (m_currentScene && m_currentScene->IsActive())
			m_currentScene->LateUpdate(deltaTime);
	}

	void SleeplessEngine::Render() {
		m_renderer->Clear();

		if (m_currentScene && m_currentScene->IsActive())
			m_currentScene->Render();

		m_renderer->Present();
	}

	void SleeplessEngine::GarbageCollect() {
		// Scene-owned destruction queues
	}

	void SleeplessEngine::Shutdown() {
		m_isRunning = false;

		if (m_currentScene)
			m_currentScene->Unload();

		m_assetManager.reset();
		m_renderer.reset();
		m_window.reset();

		SDL_Quit();

		m_isInitialized = false;
	}

} // namespace Engine
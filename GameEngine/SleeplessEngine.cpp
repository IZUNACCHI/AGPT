#include "SleeplessEngine.h"
#include <SDL3/SDL.h>
#include "Input.h"
#include "SceneManager.h"
#include "Time.h"
#include "AssetManager.h"

SleeplessEngine::SleeplessEngine() {
}

SleeplessEngine::~SleeplessEngine() {
	Shutdown();
}

void SleeplessEngine::Start(const std::string& title, int w, int h)
{
	// reset shutdown flag
	shutdownCalled = false;

	std::cout << "SleeplessEngine Initializing\n";

	// Initialize SDL, swap if change DISTRIBUTION
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		throw EngineException(SDL_GetError());
	}

	// Initialize subsystems
	std::cout << "\t-Window\n";
	window = std::make_unique<Window>(title, w, h);
	std::cout << "\t-Renderer\n";
	renderer = std::make_unique<Renderer>(*window);
	std::cout << "\t-Time\n";
	time = std::make_unique<Time>();
	std::cout << "\t-Asset Manager\n";
	assets = std::make_unique<AssetManager>(*renderer);
	std::cout << "\t-Input System\n";
	Input::Initialize();
	std::cout << "\t-Physics System\n";
	
	std::cout << "\t-Scene Manager\n";
	SceneManager::Initialize();

	std::cout << "\n";

	// Main loop
	running = true;
	std::cout << "SleeplessEngine Running\n";

	while (running) {
		time->Update();
		HandleEvents();
		if (Input::Close()) {
			running = false;
			continue;
		}
		Update();
		Render();
	}

	Shutdown();
}

// 
void SleeplessEngine::Update() {
	Scene* scene = SceneManager::GetActiveScene();
	if(scene) {
		scene->Update(time->DeltaTime());
	}
}

// Fixed timestep physics update
void SleeplessEngine::PhysicsUpdate() {
	float dt = time->DeltaTime();
	physAccumulator += dt;

	//Fixed TimeStep
	while (physAccumulator >= physStep) {
		//physics.Step(physStep);
		physAccumulator -= physStep;
	}
}

// Render the current scene
void SleeplessEngine::Render() {
	Scene* scene = SceneManager::GetActiveScene();
	if (scene)
	{
		renderer->Clear();
		scene->Render(*renderer);
		renderer->Present();
	}
}

// Handle input and window events
void SleeplessEngine::HandleEvents() {
	Input::PollEvents();
}

// Shutdown and clean up resources
void SleeplessEngine::Shutdown()
{
	// Prevent multiple shutdowns
	if (shutdownCalled) return;
	shutdownCalled = true;

	std::cout << "SleeplessEngine is Shutting Down\n";

	// Clean up subsystems
	if (renderer) renderer.reset();
	if (window) window.reset();
	if (assets) assets->Clear();
	if (assets) assets.reset();
	if (time) time.reset();
	Input::Shutdown();


	SDL_Quit();
}


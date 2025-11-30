#include "SleeplessEngine.h"
#include <SDL3/SDL.h>
#include "Input.h"
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
	Time::Initialize();
	std::cout << "\t-Asset Manager\n";
	AssetManager::Initialize(*renderer);
	std::cout << "\t-Input System\n";
	Input::Initialize();
	std::cout << "\t-Physics System\n";

	std::cout << "\t-Scene Manager\n";
	SceneManager::Initialize();
	//Load initial scene
	std::cout << "\n";

	// Main loop
	running = true;
	std::cout << "SleeplessEngine Running\n";

}



void SleeplessEngine::Run()
{
	static float deltaTime = Time::DeltaTime();
	static int fps = 0;
	while (running) {
		Time::Update();
		deltaTime = Time::DeltaTime();
		fps = static_cast<int>(1.0f / deltaTime);
		std::cout << "FPS: " << fps << std::endl;
		HandleEvents();
		if (Input::Close()) {
			running = false;
			continue;
		}
		PhysicsUpdate(deltaTime);
		Update(deltaTime);
		Render();
	}

	Shutdown();
}

// 
void SleeplessEngine::Update(float deltaTime) {
	Scene* scene = SceneManager::GetActiveScene();
	if(scene) {
		scene->Update(deltaTime);
	}
}

// Fixed timestep physics update
void SleeplessEngine::PhysicsUpdate(float deltaTime) {
	
	physAccumulator += deltaTime;
	int maxSubSteps = 10;
	int subSteps = 0;
	//Fixed TimeStep
	while (physAccumulator >= physStep && subSteps < maxSubSteps) {
		
		physAccumulator -= physStep;
		subSteps++;
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
	AssetManager::Shutdown();
	Time::Shutdown();
	Input::Shutdown();


	SDL_Quit();
}


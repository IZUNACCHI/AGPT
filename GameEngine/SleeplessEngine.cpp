#include "SleeplessEngine.h"
#include <SDL3/SDL.h>
#include "Input.h"

SleeplessEngine::SleeplessEngine() {}

SleeplessEngine::~SleeplessEngine() {
	Shutdown();
}

void SleeplessEngine::Start(const std::string& title, int w, int h)
{
	std::cout << "SleeplessEngine Initializing\n";
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) == 0) {
		throw EngineException(SDL_GetError());
	}

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
	

	std::cout << "\n";

	running = true;
	std::cout << "SleeplessEngine Running\n";

	while (running) {
		time->Update();
		HandleEvents();
		if (Input::ShouldQuit()) {
			running = false;
			continue;
		}
		Update();
		Render();
	}

	Shutdown();
}


void SleeplessEngine::Update() {
	
}

void SleeplessEngine::PhysicsUpdate() {
	float dt = time->DeltaTime();
	physAccumulator += dt;

	//Fixed TimeStep
	while (physAccumulator >= physStep) {
		//physics.Step(physStep);
		physAccumulator -= physStep;
	}
}

void SleeplessEngine::Render() {
	renderer->Clear();
	// draw stuff here
	renderer->Present();
}

void SleeplessEngine::HandleEvents() {
	Input::PollEvents();
}



void SleeplessEngine::Shutdown()
{
	std::cout << "SleeplessEngine is Shutting Down\n";
	renderer.reset();
	window.reset();
	time.reset();
	assets->Clear();

	SDL_Quit();
}


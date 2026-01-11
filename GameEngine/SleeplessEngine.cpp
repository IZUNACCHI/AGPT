// SleeplessEngine.cpp
#include "SleeplessEngine.h"
#include "Window.h"
#include "Renderer.h"
#include "Input.h"
#include "Time.hpp"
#include "Log.h"
#include "Scene.h"


struct SleeplessEngine::Impl {
	Window   window;
	Renderer renderer;
	bool     running = false;
	Scene scene; //loaded level 

	Impl(const std::string& title, int w, int h) : window(title, w, h) , renderer(window){
		Log::info("SleeplessEngine initializing");
		Log::info("- Input");
		Input::Initialize();
		Log::info("- Time");
		Time::SetTargetFPS(60);
		Time::SetFixedTimestep(1.0f / 60.0f);
		Log::info("- Physics");
		running = true;
	}

	~Impl() {
		Log::info("SleeplessEngine in shutdown");
		Input::Shutdown();
		Log::info("SleeplessEngine shutdown complete");
	}

	Impl() = default;
};

//Initiate the engine subsystems and create window
void SleeplessEngine::Start(const InitParams& params){
	impl = std::make_unique<Impl>(params.title, params.width, params.height);

	if (params.startingScene)
		LoadScene(params.startingScene);
}

void SleeplessEngine::Run() {
	if (!impl) {
		Log::error("Sleepless Engine not Initialized. Please use Start() to initialize");
		return;
	}

	Log::info("Sleepless Engine is now Running");

	while (impl->running) {
		// Update time and frame stats, time calc
		Time::Update();       

		//Process Input
		Input::PollEvents();
		// Check for quit application
		if (Input::ShouldQuit() || Input::IsKeyPressed(Key::Escape)) {
			impl->running = false;
			break;
		}
		// Toggle debug overlay
		if (Input::IsKeyPressed(Key::F3)) {
			Log::SetDebugEnabled(!Log::IsDebugEnabled());
			Log::info(std::string("Debug Overlay ") + (Log::IsDebugEnabled() ? "Enabled" : "Disabled"));
		}
		
		//fixed update
		impl->scene.Update(Time::FixedDeltaTime() * Time::FixedStepsThisFrame());

		// Clear & draw
		impl->renderer.Clear();


		impl->renderer.Present();
		Time::WaitIfNeeded();              // respects target FPS
	}

	Log::info("Sleepless Engine has Stopped Running");
}

void SleeplessEngine::Shutdown() {
	impl.reset();
}

Window& SleeplessEngine::GetWindow() { return impl->window; }
Renderer& SleeplessEngine::GetRenderer() { return impl->renderer; }
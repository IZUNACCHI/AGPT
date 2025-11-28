#pragma once
#include <iostream>
#include "Window.h"
#include "Renderer.h"
#include "EngineException.h"
#include "Time.hpp"

#include "AssetManager.h"

#include "SceneManager.h"

class SleeplessEngine
{
public:
	SleeplessEngine();
	~SleeplessEngine();

	void Start(const std::string& title, int w, int h);

	void Run();

	void Shutdown();

	// Accessors for engine systems
	Window& GetWindow() { return *window; }
	Renderer& GetRenderer() { return *renderer; }
	//Scene& GetScene() { return *scene; } 

	static SleeplessEngine* instance;

private:
	bool running = false;
	std::unique_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;

	float physStep = 1.0f / 60.0f; //fixed step for physics
	float physAccumulator = 0.0f;


	void HandleEvents();
	void Update();
	void PhysicsUpdate();
	void Render();

	// To prevent multiple shutdowns
	bool shutdownCalled = false;
};


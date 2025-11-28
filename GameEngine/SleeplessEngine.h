#pragma once
#include <iostream>
#include "Window.h"
#include "Renderer.h"
#include "EngineException.h"
#include "Time.hpp"

#include "AssetManager.h"

class SleeplessEngine
{
public:
	SleeplessEngine();
	~SleeplessEngine();

	void Start(const std::string& title, int w, int h);
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
	std::unique_ptr<Time> time;
	std::unique_ptr<AssetManager> assets;

	float physStep = 1.0f / 60.0f; //fixed step for physics
	float physAccumulator = 0.0f;


	void HandleEvents();
	void Update();
	void PhysicsUpdate();
	void Render();

	// To prevent multiple shutdowns
	bool shutdownCalled = false;
};


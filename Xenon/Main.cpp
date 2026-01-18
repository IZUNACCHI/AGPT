#include <iostream>
#include "../GameEngine/SleeplessEngine.h"


int main() {
	auto& engine = Engine::SleeplessEngine::GetInstance();

	// Simple config without initial scene
	Engine::Config config;
	config.assetBasePath = "Dist/graphics/";

	engine.Initialize(config);

	// User loads scene manually after initialization
	Engine::Scene mainScene("MainScene");
	engine.SetScene(&mainScene);

	// User would typically call engine.Run() here
	// engine.Run();

	return 0;
}
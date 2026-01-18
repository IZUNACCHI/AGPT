#include <iostream>
#include "../GameEngine/SleeplessEngine.h"


int main() {
	
	Engine::SleeplessEngine& engine = Engine::SleeplessEngine::GetInstance();
	Engine::Config startConfig;
	startConfig.assetBasePath = "Dist/graphics/";
	startConfig.windowConfig.title = "Xenon 2000";
	startConfig.windowConfig.windowSize = Engine::Vector2i(800, 640);
	engine.Initialize(Engine::Config{});
	return 0;
}
#include <iostream>
#include "../GameEngine/SleeplessEngine.h"


int main() {
	SleeplessEngine& engine = SleeplessEngine::GetInstance();
	Config startConfig;
	startConfig.assetBasePath = "Dist/graphics/";
	startConfig.windowConfig.title = "Xenon 2000";
	startConfig.windowConfig.windowSize = Vector2i(800, 640);
	engine.Initialize(startConfig);
	Scene mainScene("MainScene");
	engine.SetScene(&mainScene);
	engine.Run();
	return 0;

}
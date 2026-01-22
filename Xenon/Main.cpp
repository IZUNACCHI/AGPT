#pragma once
#include <iostream>
#include "../GameEngine/GameEngine.h"
#include "Level1.hpp"
#include <filesystem>


int main(int argc, char** argv) {
	SleeplessEngine& engine = SleeplessEngine::GetInstance();
	Config startConfig;

	startConfig.assetBasePath = "..\\Dist\\graphics";
	startConfig.windowConfig.title = "Xenon 2000";
	startConfig.windowConfig.windowSize = Vector2i(800, 640);
	startConfig.windowConfig.fullscreen = false;
	startConfig.windowConfig.borderless = false;
	startConfig.windowConfig.resizable = false;
	engine.Initialize(startConfig);
	Level1 level1;
	engine.SetScene(&level1);
	engine.Run();
	return 0;

}
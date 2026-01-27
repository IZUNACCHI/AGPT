#pragma once
#include <iostream>
#include "../GameEngine/GameEngine.h"
#include "Level1.hpp"
#include <filesystem>


int main(int argc, char** argv) {
	Config startConfig;

	startConfig.assetBasePath = "..\\Dist\\graphics";
	startConfig.windowConfig.title = "Xenon 2000";
	startConfig.windowConfig.windowSize = Vector2i(1280, 720);
	startConfig.windowConfig.fullscreen = false;
	startConfig.windowConfig.borderless = false;
	startConfig.windowConfig.resizable = false;
	startConfig.virtualResolution = Vector2(640, 480);
	startConfig.letterbox = true;
	startConfig.integerScale = true;
	startConfig.textureScaleMode = TextureScaleMode::Nearest;
	auto& engine = SleeplessEngine::GetInstance();
	engine.Initialize(startConfig);
	Level1 level1;
	engine.SetScene(&level1);
	engine.Run();
	return 0;

}
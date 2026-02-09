#pragma once
#include <iostream>
#include <GameEngine/GameEngine.h>
#include "Level1.hpp"
#include "MainMenuScene.hpp"
#include "XenonGameInstance.hpp"
#include <filesystem>


int main(int argc, char** argv) {
	Config startConfig;

	startConfig.assetBasePath = "..\\Dist\\graphics";
	startConfig.windowConfig.title = "Xenon 2000";
	startConfig.windowConfig.windowSize = Vector2i(1280, 720);
	startConfig.windowConfig.fullscreen = false;
	startConfig.windowConfig.borderless = false;
	startConfig.windowConfig.resizable = true;
	startConfig.virtualResolution = Vector2(640, 480);
	startConfig.integerScale = false;
	startConfig.debugDrawColliders = true;
	startConfig.fitWindowToScale = true;
	startConfig.viewportScaleMode = ViewportScaleMode::Letterbox;
	startConfig.textureScaleMode = TextureScaleMode::Nearest;
	auto& engine = SleeplessEngine::GetInstance();
	engine.SetGameInstanceType<XenonGameInstance>();
	engine.Initialize(startConfig);

	// Scenes are stack-allocated and live for the duration of main.
	MainMenuScene mainMenu;
	Level1 level1;

	if (auto* gi = engine.GetGameInstanceAs<XenonGameInstance>()) {
		gi->RegisterScenes(&mainMenu, &level1);
		gi->ApplySettings();
	}

	engine.SetScene(&mainMenu);
	engine.Run();
	return 0;

}
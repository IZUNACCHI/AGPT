#pragma once

#include "../GameEngine/GameEngine.h"

#include "XenonGameInstance.hpp"

#include "OptionsMenuController.hpp"

// Main menu scene.
// - Logo sprite (placeholder texture name)
// - Start -> Level1
// - Options -> opens Options menu (volume + resolution)
// - Quit -> exits game
class MainMenuScene : public Scene {
public:
	MainMenuScene()
		: Scene("MainMenu") {
		// Menus don't need special game mode.
	}

	void OnStart() override {
		// Background (optional)
		{
			auto bg = CreateGameObject<GameObject>("MenuBG");
			auto sr = bg->AddComponent<SpriteRenderer>();
			sr->SetTexture(LoadTexture("galaxy2.bmp"));
			sr->SetLayerOrder(-50);
			bg->GetTransform()->SetPosition(Vector2f(0.0f, 0.0f));
		}

		// Logo sprite (placeholder).
		// Put a logo texture in Dist/graphics with this name.
		{
			auto logo = CreateGameObject<GameObject>("Logo");
			auto sr = logo->AddComponent<SpriteRenderer>();
			sr->SetTexture(LoadTexture("Xlogo.bmp", Vector3i(255, 0, 255)));
			sr->SetLayerOrder(1000);
			logo->GetTransform()->SetPosition(Vector2f(0.0f, 140.0f));
		}

		// Root UI canvas
		auto ui = CreateGameObject<GameObject>("MainMenuUI");
		auto canvas = ui->AddComponent<UICanvas>();
		canvas->SetSortingOrder(0);

		// Shared style
		UIButtonStyle st;
		st.font = LoadBitmapFont("Font16x16.bmp", Vector2i(16, 16), Vector3i(255, 0, 255)); 
		st.bgNormal = { 0, 0, 0, 0 };
		st.bgHover = { 60, 60, 60, 240 };
		st.bgPressed = { 20, 20, 20, 240 };
		st.bgFocused = { 70, 70, 70, 240 };
		st.textColor = { 235, 235, 235, 255 };
		st.borderColor = { 200, 200, 200, 255 };
		st.borderThickness = 0.0f;
		st.padding = 10.0f;

		const float cx = 640.0f * 0.5f;
		const float btnW = 260.0f;
		const float btnH = 48.0f;
		float y = 210.0f;

		auto makeBtn = [&](const char* name, const char* text, int order) -> std::shared_ptr<UIButton> {
			auto go = CreateGameObject<GameObject>(name);
			go->GetTransform()->SetPosition(Vector2f(0, 0));
			auto b = go->AddComponent<UIButton>();
			b->SetRect({ cx - btnW * 0.5f, y, btnW, btnH });
			b->SetOrderInCanvas(order);
			b->SetStyle(st);
			b->SetText(text);
			y += 64.0f;
			return b;
		};

		auto start = makeBtn("StartBtn", "Start (1P)", 0);
		auto options = makeBtn("OptionsBtn", "Options", 1);
		auto quit = makeBtn("QuitBtn", "Quit", 2);

		// Options menu GameObject (hidden by default)
		auto optionsGO = CreateGameObject<GameObject>("OptionsMenu");
		auto opts = optionsGO->AddComponent<OptionsMenuController>();
		optionsGO->SetActive(false);
		opts->SetOnClose([optionsGO, options]() {
			optionsGO->SetActive(false);
			UISystem::Get().SetFocused(options.get());
		});

		// Hook callbacks
		start->SetOnClick([this]() {
			auto* gi = SleeplessEngine::GetInstance().GetGameInstanceAs<XenonGameInstance>();
			if (gi) gi->StartLevel1();
		});

		options->SetOnClick([optionsGO]() {
			optionsGO->SetActive(true);
		});

		quit->SetOnClick([]() {
			SleeplessEngine::GetInstance().Shutdown();
		});

		// Default focus
		UISystem::Get().SetFocused(start.get());
	}
};

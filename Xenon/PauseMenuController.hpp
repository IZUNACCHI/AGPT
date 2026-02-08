#pragma once

#include <GameEngine/GameEngine.h>
#include "XenonGameInstance.hpp"
#include "OptionsMenuController.hpp"

// Adds a pause menu overlay to any gameplay scene (no scene switch).
//
// Controls:
// - Toggle pause: Escape (keyboard) or Start (gamepad)
//
// Buttons:
// - Resume
// - Options (opens OptionsMenuController)
// - Quit to Title
// - Quit Game
class PauseMenuController final : public MonoBehaviour {
public:
	PauseMenuController() { SetName("PauseMenuController"); }

protected:
	void Start() override {
		BuildUI();
		HideAll();
	}

	void Update() override {
		// Toggle pause
		if (Input::IsKeyPressed(Key::Escape) || Input::IsGamepadButtonPressed(GamepadButton::Start, 0)) {
			if (m_isPaused) Resume(); else Pause();
		}
	}

private:
	XenonGameInstance* GI() const {
		return SleeplessEngine::GetInstance().GetGameInstanceAs<XenonGameInstance>();
	}

	void BuildUI() {
		int vw = 640, vh = 480;
		UISystem::GetVirtualUISize(vw, vh);
		const float cx = vw * 0.5f;
		const float btnW = 260.0f;
		const float btnH = 48.0f;
		float y = 150.0f;

		// Root GO for pause menu
		auto* scene = GetGameObject()->GetScene();
		m_pauseRoot = scene->CreateGameObject<GameObject>("PauseMenuUI");
		m_pauseRoot->GetTransform()->SetParent(GetTransform());
		m_pauseRoot->AddComponent<UICanvas>()->SetSortingOrder(100);

		// Screen dim (drawn behind the buttons).
		{
			auto dimGO = scene->CreateGameObject<GameObject>("PauseDimOverlay");
			dimGO->GetTransform()->SetParent(m_pauseRoot->GetTransform());
			auto dim = dimGO->AddComponent<UIPanel>();
			dim->SetRect({ 0.0f, 0.0f, (float)vw, (float)vh });
			dim->SetOrderInCanvas(-1000);
			UIPanelStyle ps;
			ps.color = { 0, 0, 0, 120 }; // subtle darken
			dim->SetStyle(ps);
		}

		UIButtonStyle st;
		st.font = LoadBitmapFont("font16x16.bmp", Vector2i(16, 16));
		st.bgNormal = { 40, 40, 40, 220 };
		st.bgHover = { 60, 60, 60, 240 };
		st.bgPressed = { 20, 20, 20, 240 };
		st.bgFocused = { 70, 70, 70, 240 };
		st.textColor = { 235, 235, 235, 255 };
		st.borderColor = { 200, 200, 200, 255 };
		st.borderThickness = 2.0f;
		st.padding = 10.0f;

		auto makeBtn = [&](const char* name, const char* text, int order) -> std::shared_ptr<UIButton> {
			auto go = scene->CreateGameObject<GameObject>(name);
			go->GetTransform()->SetParent(m_pauseRoot->GetTransform());
			auto b = go->AddComponent<UIButton>();
			b->SetRect({ cx - btnW * 0.5f, y, btnW, btnH });
			b->SetOrderInCanvas(order);
			b->SetStyle(st);
			b->SetText(text);
			y += 64.0f;
			return b;
		};

		m_resume = makeBtn("PauseResume", "Resume", 0);
		m_optionsBtn = makeBtn("PauseOptions", "Options", 1);
		m_quitTitle = makeBtn("PauseQuitTitle", "Quit to Title", 2);
		m_quitGame = makeBtn("PauseQuitGame", "Quit Game", 3);

		// Options menu GO
		auto optionsGO = scene->CreateGameObject<GameObject>("PauseOptionsMenu");
		optionsGO->GetTransform()->SetParent(GetTransform());
		m_optionsMenu = optionsGO->AddComponent<OptionsMenuController>();
		optionsGO->SetActive(false);

		m_resume->SetOnClick([this]() { Resume(); });
		m_optionsBtn->SetOnClick([this, optionsGO]() {
			// Hide pause menu so the options menu is clearly on top.
			if (m_pauseRoot) m_pauseRoot->SetActive(false);
			optionsGO->SetActive(true);
		});
		m_quitTitle->SetOnClick([this]() {
			Resume();
			if (auto* gi = GI()) gi->GoToMainMenu();
		});
		m_quitGame->SetOnClick([this]() {
			Resume();
			SleeplessEngine::GetInstance().Shutdown();
		});

		// Navigation explicit
		m_resume->down = m_optionsBtn.get();
		m_optionsBtn->up = m_resume.get();
		m_optionsBtn->down = m_quitTitle.get();
		m_quitTitle->up = m_optionsBtn.get();
		m_quitTitle->down = m_quitGame.get();
		m_quitGame->up = m_quitTitle.get();

		// Close behaviour for options: return focus to pause menu.
		m_optionsMenu->SetOnClose([this, optionsGO]() {
			optionsGO->SetActive(false);
			if (m_isPaused) {
				if (m_pauseRoot) m_pauseRoot->SetActive(true);
				UISystem::Get().SetFocused(m_resume.get());
			}
		});
	}

	void HideAll() {
		if (m_pauseRoot) m_pauseRoot->SetActive(false);
		if (m_optionsMenu && m_optionsMenu->GetGameObject()) m_optionsMenu->GetGameObject()->SetActive(false);
	}

	void Pause() {
		m_isPaused = true;
		Time::SetTimeScale(0.0f);
		if (m_pauseRoot) m_pauseRoot->SetActive(true);
		UISystem::Get().SetFocused(m_resume.get());
	}

	void Resume() {
		m_isPaused = false;
		Time::SetTimeScale(1.0f);
		HideAll();
	}

private:
	bool m_isPaused = false;
	std::shared_ptr<GameObject> m_pauseRoot;
	std::shared_ptr<OptionsMenuController> m_optionsMenu;
	std::shared_ptr<UIButton> m_resume;
	std::shared_ptr<UIButton> m_optionsBtn;
	std::shared_ptr<UIButton> m_quitTitle;
	std::shared_ptr<UIButton> m_quitGame;
};

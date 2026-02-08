#pragma once

#include <GameEngine/GameEngine.h>
#include "XenonGameInstance.hpp"

// A reusable options menu that can be spawned in any scene.
//
// Features:
// - Master volume (0..1) with -/+ buttons + progress bar
// - Resolution selection (cycles a small preset list)
// - Back button to close
//
// This menu is "sceneless" in the sense that it's just a GameObject you can
// show/hide in any scene.
class OptionsMenuController final : public MonoBehaviour {
public:
	OptionsMenuController() { SetName("OptionsMenuController"); }

	// Optional: provide a callback invoked when "Back" is pressed.
	void SetOnClose(std::function<void()> fn) { m_onClose = std::move(fn); }

protected:
	void Start() override {
		BuildUI();
		RefreshLabels();
	}

	void OnEnable() override {
		// When the menu opens, focus the first interactive button.
		if (m_volumeMinus) {
			UISystem::Get().SetFocused(m_volumeMinus.get());
		}
	}

private:
	XenonGameInstance* GI() const {
		return SleeplessEngine::GetInstance().GetGameInstanceAs<XenonGameInstance>();
	}

	void BuildUI() {
		auto* scene = GetGameObject()->GetScene();
		auto makeChild = [&](const std::string& name) {
			auto go = scene->CreateGameObject<GameObject>(name);
			go->GetTransform()->SetParent(GetTransform());
			return go;
		};

		int vw = 640, vh = 480;
		UISystem::GetVirtualUISize(vw, vh);
		const float cx = vw * 0.5f;

		// Canvas
		auto canvas = GetGameObject()->AddComponent<UICanvas>();
		// Must be above the pause menu overlay.
		canvas->SetSortingOrder(120);

		UIButtonStyle st;
		st.font = LoadBitmapFont("Font8x8.bmp", Vector2i(8, 8), Vector3i(255, 0, 255));
		st.bgNormal = { 20, 20, 20, 230 };
		st.bgHover = { 45, 45, 45, 240 };
		st.bgPressed = { 10, 10, 10, 240 };
		st.bgFocused = { 60, 60, 60, 240 };
		st.textColor = { 235, 235, 235, 255 };
		st.borderColor = { 200, 200, 200, 255 };
		st.borderThickness = 2.0f;
		st.padding = 10.0f;

		// Semi-transparent backdrop panel (non-interactable)
		{
			// Full-screen input blocker. This prevents hover/click from affecting
			// UI that sits behind this menu (e.g., main menu buttons).
			auto blockerGO = makeChild("OptionsInputBlocker");
			blockerGO->SetLayer(9999);
			auto blocker = blockerGO->AddComponent<UIPanel>();
			blocker->SetRect({ 0.0f, 0.0f, (float)vw, (float)vh });
			blocker->SetOrderInCanvas(-200);
			UIPanelStyle bs;
			bs.color = { 0, 0, 0, 70 }; // subtle dim
			bs.showBorder = false;
			blocker->SetStyle(bs);

			auto panelGO = makeChild("OptionsPanel");
			panelGO->SetLayer(9999);
			auto panel = panelGO->AddComponent<UIButton>();
			panel->SetRect({ cx - 220.0f, 55.0f, 440.0f, 420.0f });
			panel->SetOrderInCanvas(-100);
			panel->SetStyle(st);
			panel->SetText("OPTIONS");
			panel->SetInteractable(false);
			// Make it look like a panel: use normal color, no hover changes.
			UIButtonStyle pst = st;
			pst.bgNormal = { 0, 0, 0, 170 };
			pst.borderColor = { 180, 180, 180, 220 };
			panel->SetStyle(pst);
		}

		// Volume label (non-interactable)
		{
			auto labelGO = makeChild("VolLabel");
			auto label = labelGO->AddComponent<UIButton>();
			label->SetRect({ cx - 200.0f, 120.0f, 400.0f, 36.0f });
			label->SetOrderInCanvas(0);
			label->SetStyle(st);
			label->SetInteractable(false);
			m_volumeLabel = label;
		}

		// Volume - button
		{
			auto bgo = makeChild("VolMinus");
			m_volumeMinus = bgo->AddComponent<UIButton>();
			m_volumeMinus->SetRect({ cx - 200.0f, 165.0f, 60.0f, 40.0f });
			m_volumeMinus->SetOrderInCanvas(1);
			m_volumeMinus->SetStyle(st);
			m_volumeMinus->SetText("-");
			m_volumeMinus->SetOnClick([this]() {
				auto* gi = GI();
				if (!gi) return;
				gi->SetMasterVolume(gi->GetSettings().masterVolume - 0.05f);
				RefreshLabels();
			});
		}

		// Volume + button
		{
			auto bgo = makeChild("VolPlus");
			m_volumePlus = bgo->AddComponent<UIButton>();
			m_volumePlus->SetRect({ cx + 140.0f, 165.0f, 60.0f, 40.0f });
			m_volumePlus->SetOrderInCanvas(2);
			m_volumePlus->SetStyle(st);
			m_volumePlus->SetText("+");
			m_volumePlus->SetOnClick([this]() {
				auto* gi = GI();
				if (!gi) return;
				gi->SetMasterVolume(gi->GetSettings().masterVolume + 0.05f);
				RefreshLabels();
			});
		}

		// Volume bar
		{
			auto barGO = makeChild("VolBar");
			m_volumeBar = barGO->AddComponent<UIProgressBar>();
			m_volumeBar->SetRect({ cx - 130.0f, 175.0f, 260.0f, 20.0f });
			m_volumeBar->SetOrderInCanvas(3);
			UIProgressBarStyle ps;
			ps.backColor = { 30, 30, 30, 220 };
			ps.fillColor = { 0, 200, 90, 255 };
			ps.borderColor = { 200, 200, 200, 255 };
			ps.borderThickness = 2.0f;
			ps.showBorder = true;
			m_volumeBar->SetStyle(ps);
		}

		// Resolution label
		{
			auto labelGO = makeChild("ResLabel");
			auto label = labelGO->AddComponent<UIButton>();
			label->SetRect({ cx - 200.0f, 225.0f, 400.0f, 36.0f });
			label->SetOrderInCanvas(4);
			label->SetStyle(st);
			label->SetInteractable(false);
			m_resLabel = label;
		}

		// Res prev
		{
			auto bgo = makeChild("ResPrev");
			m_resPrev = bgo->AddComponent<UIButton>();
			m_resPrev->SetRect({ cx - 200.0f, 270.0f, 180.0f, 44.0f });
			m_resPrev->SetOrderInCanvas(5);
			m_resPrev->SetStyle(st);
			m_resPrev->SetText("Prev");
			m_resPrev->SetOnClick([this]() {
				auto* gi = GI();
				if (!gi) return;
				gi->SetResolutionIndex(gi->GetSettings().resolutionIndex - 1);
				RefreshLabels();
			});
		}

		// Res next
		{
			auto bgo = makeChild("ResNext");
			m_resNext = bgo->AddComponent<UIButton>();
			m_resNext->SetRect({ cx + 20.0f, 270.0f, 180.0f, 44.0f });
			m_resNext->SetOrderInCanvas(6);
			m_resNext->SetStyle(st);
			m_resNext->SetText("Next");
			m_resNext->SetOnClick([this]() {
				auto* gi = GI();
				if (!gi) return;
				gi->SetResolutionIndex(gi->GetSettings().resolutionIndex + 1);
				RefreshLabels();
			});
		}

		// Fullscreen label
		{
			auto labelGO = makeChild("FullscreenLabel");
			auto label = labelGO->AddComponent<UIButton>();
			label->SetRect({ cx - 200.0f, 325.0f, 400.0f, 36.0f });
			label->SetOrderInCanvas(7);
			label->SetStyle(st);
			label->SetInteractable(false);
			m_fullscreenLabel = label;
		}

		// Fullscreen toggle
		{
			auto bgo = makeChild("FullscreenToggle");
			m_fullscreenToggle = bgo->AddComponent<UIButton>();
			m_fullscreenToggle->SetRect({ cx - 130.0f, 365.0f, 260.0f, 44.0f });
			m_fullscreenToggle->SetOrderInCanvas(8);
			m_fullscreenToggle->SetStyle(st);
			m_fullscreenToggle->SetText("Toggle Fullscreen");
			m_fullscreenToggle->SetOnClick([this]() {
				auto* gi = GI();
				if (!gi) return;
				gi->SetFullscreen(!gi->IsFullscreen());
				RefreshLabels();
			});
		}

		// Back
		{
			auto bgo = makeChild("OptionsBack");
			m_back = bgo->AddComponent<UIButton>();
			m_back->SetRect({ cx - 130.0f, 415.0f, 260.0f, 44.0f });
			m_back->SetOrderInCanvas(9);
			m_back->SetStyle(st);
			m_back->SetText("Back");
			m_back->SetOnClick([this]() {
				if (m_onClose) {
					m_onClose();
				}
				// Default behavior: hide the menu.
				GetGameObject()->SetActive(false);
			});
		}

		// Basic navigation wiring (vertical)
		if (m_volumeMinus && m_volumePlus) {
			m_volumeMinus->right = m_volumePlus.get();
			m_volumePlus->left = m_volumeMinus.get();
		}
		if (m_resPrev && m_resNext) {
			m_resPrev->right = m_resNext.get();
			m_resNext->left = m_resPrev.get();
		}

		// Down-chain
		if (m_volumeMinus && m_resPrev) {
			m_volumeMinus->down = m_resPrev.get();
			m_volumePlus->down = m_resNext.get();
		}
		if (m_resPrev && m_fullscreenToggle) {
			m_resPrev->down = m_fullscreenToggle.get();
			m_resNext->down = m_fullscreenToggle.get();
			m_fullscreenToggle->up = m_resPrev.get();
		}
		if (m_fullscreenToggle && m_back) {
			m_fullscreenToggle->down = m_back.get();
			m_back->up = m_fullscreenToggle.get();
		}
	}

	void RefreshLabels() {
		auto* gi = GI();
		if (!gi) return;

		const int volPct = (int)(gi->GetSettings().masterVolume * 100.0f + 0.5f);
		if (m_volumeLabel) {
			m_volumeLabel->SetText(std::string("Volume: ") + std::to_string(volPct) + "%");
		}
		if (m_volumeBar) {
			m_volumeBar->SetValue(gi->GetSettings().masterVolume);
		}
		if (m_resLabel) {
			Vector2i r = gi->GetCurrentResolution();
			m_resLabel->SetText(std::string("Resolution: ") + std::to_string(r.x) + "x" + std::to_string(r.y));
		}
		if (m_fullscreenLabel) {
			m_fullscreenLabel->SetText(std::string("Fullscreen: ") + (gi->IsFullscreen() ? "On" : "Off"));
		}
	}

private:
	std::function<void()> m_onClose;

	std::shared_ptr<UIButton> m_volumeLabel;
	std::shared_ptr<UIButton> m_resLabel;
	std::shared_ptr<UIButton> m_volumeMinus;
	std::shared_ptr<UIButton> m_volumePlus;
	std::shared_ptr<UIProgressBar> m_volumeBar;
	std::shared_ptr<UIButton> m_resPrev;
	std::shared_ptr<UIButton> m_resNext;
	std::shared_ptr<UIButton> m_fullscreenLabel;
	std::shared_ptr<UIButton> m_fullscreenToggle;
	std::shared_ptr<UIButton> m_back;
};

#pragma once

#include <GameEngine/GameEngine.h>
#include <GameEngine/UILabel.h>
#include <GameEngine/UIImage.h>
#include <GameEngine/UIProgressBar.h>

#include "XenonGameMode.hpp"

#include <cstdio>

// Level HUD matching the reference screenshot:
// - Bottom-left: health bar
// - Above health bar: lives (Ship2.bmp frame 4)
// - Top-left: "Player One" (Font8x8.bmp) + score (Font16x16.bmp)
// - Top-center: "Hi Score" (Font8x8.bmp) + hi score value (Font16x16.bmp)
class XenonHUDController final : public MonoBehaviour {
public:
	XenonHUDController() { SetName("XenonHUDController"); }

protected:
	void Start() override {
		BuildHUD();
	}

	void Update() override {
		auto* gm = GetMode();
		if (!gm) return;

		// Health
		if (m_healthBar) {
			float v = 1.0f;
			int mh = gm->GetMaxHealth();
			if (mh > 0) v = (float)gm->GetHealth() / (float)mh;
			m_healthBar->SetValue(v);
		}

		// Score + hi score
		if (m_scoreValue) m_scoreValue->SetText(FormatScore(gm->GetScore()));
		if (m_hiScoreValue) m_hiScoreValue->SetText(FormatScore(gm->GetHiScore()));

		// Lives icons
		UpdateLives(gm->GetLives());
	}

private:
	XenonGameMode* GetMode() const {
		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return nullptr;
		return dynamic_cast<XenonGameMode*>(scene->GetGameMode());
	}

	static std::string FormatScore(int s) {
		if (s < 0) s = 0;
		char buf[32]{};
		// 8 digits, zero-padded (matches the reference vibe)
		snprintf(buf, sizeof(buf), "%08d", s);
		return std::string(buf);
	}

	void BuildHUD() {
		int vw = 640, vh = 480;
		UISystem::GetVirtualUISize(vw, vh);
		m_virtualW = vw;
		m_virtualH = vh;

		Scene* scene = GetGameObject()->GetScene();
		if (!scene) return;

		// Root canvas
		m_root = scene->CreateGameObject<GameObject>("HUD_UI");
		m_root->GetTransform()->SetParent(GetTransform());
		auto canvas = m_root->AddComponent<UICanvas>();
		canvas->SetSortingOrder(20); // below pause (100), above world

		// Fonts (color key 255,0,255)
		BitmapFont* font8 = LoadBitmapFont("Font8x8.bmp", Vector2i(8, 8), Vector3i(255, 0, 255));
		BitmapFont* font16 = LoadBitmapFont("Font16x16.bmp", Vector2i(16, 16), Vector3i(255, 0, 255));

		// --- Top-left labels ---
		{
			auto p1GO = scene->CreateGameObject<GameObject>("HUD_PlayerOne");
			p1GO->GetTransform()->SetParent(m_root->GetTransform());
			auto label = p1GO->AddComponent<UILabel>();
			label->SetFont(font8);
			label->SetText("Player One");
			label->SetRect({ 10.0f, 6.0f, 0.0f, 0.0f });
			label->SetAnchor(UILabelAnchor::TopLeft);
			label->SetColor({ 255, 255, 255, 255 });
			label->SetOrderInCanvas(0);
		}
		{
			auto scoreGO = scene->CreateGameObject<GameObject>("HUD_Score");
			scoreGO->GetTransform()->SetParent(m_root->GetTransform());
			m_scoreValue = scoreGO->AddComponent<UILabel>().get();
			m_scoreValue->SetFont(font16);
			m_scoreValue->SetText("00000000");
			m_scoreValue->SetRect({ 10.0f, 18.0f, 0.0f, 0.0f });
			m_scoreValue->SetAnchor(UILabelAnchor::TopLeft);
			m_scoreValue->SetColor({ 255, 255, 255, 255 });
			m_scoreValue->SetOrderInCanvas(1);
		}

		// --- Top-center hi score ---
		{
			const float cx = vw * 0.5f;
			auto hiGO = scene->CreateGameObject<GameObject>("HUD_HiScoreLabel");
			hiGO->GetTransform()->SetParent(m_root->GetTransform());
			auto hi = hiGO->AddComponent<UILabel>();
			hi->SetFont(font8);
			hi->SetText("Hi Score");
			hi->SetRect({ cx, 6.0f, 0.0f, 0.0f });
			hi->SetAnchor(UILabelAnchor::TopCenter);
			hi->SetColor({ 255, 255, 255, 255 });
			hi->SetOrderInCanvas(0);

			auto hvGO = scene->CreateGameObject<GameObject>("HUD_HiScoreValue");
			hvGO->GetTransform()->SetParent(m_root->GetTransform());
			m_hiScoreValue = hvGO->AddComponent<UILabel>().get();
			m_hiScoreValue->SetFont(font8);
			m_hiScoreValue->SetText("00000000");
			m_hiScoreValue->SetRect({ cx, 18.0f, 0.0f, 0.0f });
			m_hiScoreValue->SetAnchor(UILabelAnchor::TopCenter);
			m_hiScoreValue->SetColor({ 255, 255, 255, 255 });
			m_hiScoreValue->SetOrderInCanvas(1);
		}

		// --- Bottom-left health bar ---
		{
			const float barW = 150.0f;
			const float barH = 10.0f;
			const float x = 10.0f;
			const float y = (float)vh - 16.0f; // a bit above the bottom

			auto hbGO = scene->CreateGameObject<GameObject>("HUD_HealthBar");
			hbGO->GetTransform()->SetParent(m_root->GetTransform());
			m_healthBar = hbGO->AddComponent<UIProgressBar>().get();
			m_healthBar->SetRect({ x, y, barW, barH });
			m_healthBar->SetOrderInCanvas(10);
			UIProgressBarStyle ps;
			ps.backColor = { 10, 10, 10, 220 };
			ps.fillColor = { 0, 220, 0, 255 };
			ps.showBorder = true;
			ps.borderThickness = 1.0f;
			ps.borderColor = { 255, 255, 255, 255 };
			ps.leftToRight = true;
			m_healthBar->SetStyle(ps);
		}

		// --- Lives icons (Ship2.bmp frame 4 = center frame, col 3 row 0) ---
		m_lifeTexture = LoadTexture("Ship2.bmp", Vector3i(255, 0, 255));
		m_lifeSrcPos = Vector2f(64.0f * 3.0f, 0.0f);
		m_lifeSrcSize = Vector2f(64.0f, 64.0f);
		m_lastLives = -1;
		UpdateLives(3);
	}

	void UpdateLives(int lives) {
		if (!m_root) return;
		lives = std::max(0, lives);
		if (lives == m_lastLives) return;
		m_lastLives = lives;

		// Destroy old life icons.
		for (auto& go : m_lifeGOs) {
			if (go) Object::Destroy(go);
		}
		m_lifeGOs.clear();

		Scene* scene = GetGameObject()->GetScene();
		if (!scene || !m_lifeTexture) return;

		const float startX = 10.0f;
		const float y = (float)m_virtualH - 16.0f - 18.0f; // above health bar
		const float iconW = 32.f; 
		const float iconH = 32.f;
		const float spacing = 18.0f;

		for (int i = 0; i < lives; ++i) {
			auto lifeGO = scene->CreateGameObject<GameObject>("HUD_Life" + std::to_string(i));
			lifeGO->GetTransform()->SetParent(m_root->GetTransform());
			auto img = lifeGO->AddComponent<UIImage>();
			img->SetTexture(m_lifeTexture);
			img->SetSourceRect(m_lifeSrcPos, m_lifeSrcSize);
			img->SetRect({ startX + i * spacing, y, iconW, iconH });
			img->SetInteractable(false);
			img->SetOrderInCanvas(9);
			m_lifeGOs.push_back(lifeGO);
		}
	}

private:
	std::shared_ptr<GameObject> m_root;
	UILabel* m_scoreValue = nullptr;
	UILabel* m_hiScoreValue = nullptr;
	UIProgressBar* m_healthBar = nullptr;

	int m_virtualW = 640;
	int m_virtualH = 480;

	Texture* m_lifeTexture = nullptr;
	Vector2f m_lifeSrcPos{ 0.0f, 0.0f };
	Vector2f m_lifeSrcSize{ 64.0f, 64.0f };
	int m_lastLives = -1;
	std::vector<std::shared_ptr<GameObject>> m_lifeGOs;
};

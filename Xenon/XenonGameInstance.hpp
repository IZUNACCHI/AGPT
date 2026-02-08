#pragma once

#include <GameEngine/GameInstance.h>
#include <GameEngine/SleeplessEngine.h>
#include <GameEngine/Audio.h>
#include <GameEngine/Types.hpp>

#include <string>
#include <vector>

class Scene;

/// Game-specific GameInstance for Xenon.
/// Put long-lived state here (settings, highscores, persistent services).
class XenonGameInstance final : public GameInstance {
public:
	XenonGameInstance() = default;

	void OnInit() override {
		// TODO: load settings / highscores, initialize audio banks, etc.
	}

	void OnShutdown() override {
		// TODO: save settings / highscores.
	}

	const char* GetDebugName() const override { return "XenonGameInstance"; }

	// ---------------- Settings ----------------
	struct Settings {
		float masterVolume = 1.0f; // 0..1
		int resolutionIndex = 0;
		bool fullscreen = false;
	};

	const Settings& GetSettings() const { return m_settings; }
	void SetMasterVolume(float v) {
		if (v < 0.0f) v = 0.0f;
		if (v > 1.0f) v = 1.0f;
		m_settings.masterVolume = v;
		Audio::SetMasterGain(m_settings.masterVolume);
	}

	std::vector<Vector2i> GetSupportedResolutions() const {
		// Reasonable defaults; you can add/remove as you like.
		return {
			{1280, 720},
			{1600, 900},
			{1920, 1080},
			{800, 600},
			{1024, 768},
		};
	}

	Vector2i GetCurrentResolution() const {
		auto list = GetSupportedResolutions();
		int idx = m_settings.resolutionIndex;
		if (idx < 0) idx = 0;
		if (idx >= (int)list.size()) idx = 0;
		return list[(size_t)idx];
	}

	void SetResolutionIndex(int idx) {
		auto list = GetSupportedResolutions();
		if (list.empty()) return;
		if (idx < 0) idx = (int)list.size() - 1;
		if (idx >= (int)list.size()) idx = 0;
		m_settings.resolutionIndex = idx;

		// Apply immediately.
		auto& engine = SleeplessEngine::GetInstance();
		if (auto* w = engine.GetWindow()) {
			w->SetSize(GetCurrentResolution());
		}
	}

	bool IsFullscreen() const { return m_settings.fullscreen; }
	void SetFullscreen(bool fs) {
		m_settings.fullscreen = fs;
		auto& engine = SleeplessEngine::GetInstance();
		if (auto* w = engine.GetWindow()) {
			w->SetFullscreen(fs);
		}
	}

	void ApplySettings() {
		Audio::SetMasterGain(m_settings.masterVolume);
		auto& engine = SleeplessEngine::GetInstance();
		if (auto* w = engine.GetWindow()) {
			w->SetSize(GetCurrentResolution());
			w->SetFullscreen(m_settings.fullscreen);
		}
	}

	// Example persistent state (placeholder)
	int GetLastScore() const { return m_lastScore; }
	void SetLastScore(int score) { m_lastScore = score; }

	// ---------------- Scene switching ----------------
	void RegisterScenes(Scene* mainMenu, Scene* level1) {
		m_mainMenuScene = mainMenu;
		m_level1Scene = level1;
	}

	void GoToMainMenu() {
		if (!m_mainMenuScene) return;
		SleeplessEngine::GetInstance().SetScene(m_mainMenuScene);
	}

	void StartLevel1() {
		if (!m_level1Scene) return;
		SleeplessEngine::GetInstance().SetScene(m_level1Scene);
	}

	void QuitGame() {
		SleeplessEngine::GetInstance().Shutdown();
	}

private:
	int m_lastScore = 0;
	Settings m_settings{};
	Scene* m_mainMenuScene = nullptr;
	Scene* m_level1Scene = nullptr;
};

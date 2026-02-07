#pragma once

#include "../GameEngine/GameMode.h"
#include "../GameEngine/GameEngine.h"

#include "PauseMenuController.hpp"
#include "XenonGameInstance.hpp"
#include "Entity.hpp"
#include "SpaceShip.hpp"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

class XenonGameMode : public GameMode {
public:
	XenonGameMode() = default;

	int GetLives() const { return m_lives; }
	int GetScore() const { return m_score; }
	int GetHiScore() const { return m_hiScore; }
	int GetHealth() const { return m_health; }
	int GetMaxHealth() const { return m_maxHealth; }

	void AddScore(int amount) {
		if (amount <= 0) return;
		m_score += amount;
		if (m_score > m_hiScore) m_hiScore = m_score;
	}

	void SetLives(int lives) { m_lives = std::max(0, lives); }
	void SetScore(int score) {
		m_score = std::max(0, score);
		if (m_score > m_hiScore) m_hiScore = m_score;
	}

	void OnAttach(Scene& scene) override {
		m_scene = &scene;
		LoadHighScores();
		m_hiScore = m_highScores.empty() ? 0 : m_highScores.front();
	}

	void OnStart() override {
		if (!m_scene) return;

		{
			auto go = m_scene->CreateGameObject<GameObject>("PauseController");
			go->AddComponent<PauseMenuController>();
		}
	}

	void OnUpdate() override {
		SyncPlayerHealth();

		if (!HasPlayer()) {
			if (m_lives <= 0) {
				if (!m_gameOverHandled) {
					m_gameOverHandled = true;
					if (auto* gi = SleeplessEngine::GetInstance().GetGameInstanceAs<XenonGameInstance>()) {
						gi->GoToMainMenu();
					}
					else {
						SleeplessEngine::GetInstance().Shutdown();
					}
				}
				return;
			}

			if (!m_waitingForRespawn) {
				m_waitingForRespawn = true;
				m_lives = std::max(0, m_lives - 1);

				if (m_lives <= 0) {
					return;
				}

				m_respawnTimer = 1.0f;
			}
		}

		if (m_respawnTimer > 0.0f) {
			m_respawnTimer -= Time::DeltaTime();
			if (m_respawnTimer <= 0.0f) {
				m_respawnTimer = 0.0f;
				TryRespawn();
			}
		}
	}

	void OnDestroy() override {
		m_highScores.push_back(m_score);
		std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
		if ((int)m_highScores.size() > 10) m_highScores.resize(10);
		WriteHighScores();
	}

	const char* GetDebugName() const override { return "XenonGameMode"; }

private:
	static std::string HighScoreFilePath() {
		return "xenon_highscores.txt";
	}

	void LoadHighScores() {
		m_highScores.clear();
		std::ifstream in(HighScoreFilePath());
		if (!in.is_open()) return;
		int s = 0;
		while (in >> s) {
			m_highScores.push_back(std::max(0, s));
			if ((int)m_highScores.size() >= 1000) break;
		}
		std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
		if ((int)m_highScores.size() > 10) m_highScores.resize(10);
	}

	void WriteHighScores() {
		std::ofstream out(HighScoreFilePath(), std::ios::trunc);
		if (!out.is_open()) {
			LOG_WARN("Failed to write highscores file");
			return;
		}
		for (size_t i = 0; i < m_highScores.size(); ++i) {
			out << m_highScores[i] << "\n";
		}
	}

	bool HasPlayer() const {
		if (!m_scene) return false;
		auto go = Scene::FindGameObject("SpaceShip");
		return (go != nullptr) && go->IsActiveInHierarchy();
	}

	void SyncPlayerHealth() {
		if (!m_scene) return;

		auto playerGO = Scene::FindGameObject("SpaceShip");
		if (!playerGO) {
			m_health = 0;
			return;
		}

		auto behaviours = playerGO->GetComponents<MonoBehaviour>();
		for (const auto& b : behaviours) {
			if (!b) continue;
			if (auto* e = dynamic_cast<Entity*>(b.get())) {
				m_health = e->GetHealth();
				m_maxHealth = e->GetMaxHealth();
				return;
			}
		}
	}

	void TryRespawn() {
		if (!m_scene) return;
		if (HasPlayer()) {
			m_waitingForRespawn = false;
			return;
		}
		if (m_lives <= 0) return;

		auto ship = m_scene->CreateGameObject<SpaceShip>("SpaceShip");
		ship->GetTransform()->SetPosition(Vector2f(0.0f, -160.0f));
		ship->GetTransform()->SetRotation(-90.0f);

		m_waitingForRespawn = false;
	}

private:
	Scene* m_scene = nullptr;

	int m_lives = 3;
	int m_score = 0;
	int m_hiScore = 0;
	int m_health = 100;
	int m_maxHealth = 100;

	float m_respawnTimer = 0.0f;
	bool m_waitingForRespawn = false;
	bool m_gameOverHandled = false;

	std::vector<int> m_highScores;
};

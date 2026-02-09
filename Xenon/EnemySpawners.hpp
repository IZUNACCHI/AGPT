// Xenon/EnemySpawners.hpp
#pragma once

#include <GameEngine/GameEngine.h>

#include "Loner.hpp"
#include "Rusher.hpp"
#include <algorithm>
#include <memory>

static inline bool GetShipPose(Vector2f& outPos, Vector2f& outUp, Vector2f& outRight, float& outRot) {
	auto ship = Scene::FindGameObject("SpaceShip");
	if (!ship || !ship->IsActiveInHierarchy()) return false;
	auto t = ship->GetTransform();
	if (!t) return false;
	outPos = t->GetWorldPosition();
	outUp = t->GetUp();
	outRight = t->GetRight();
	outRot = t->GetRotation();
	return true;
}

class KeyedSpawnerBase : public MonoBehaviour {
public:
	KeyedSpawnerBase() { SetComponentName("KeyedSpawnerBase"); }

	void SetKey(Key key) { m_key = key; }
	void SetCooldown(float seconds) { m_cooldown = std::max(0.0f, seconds); }

protected:
	bool ConsumeSpawnPress() {
		if (m_key == Key::Unknown) return false;
		if (!IsKeyPressed(m_key)) return false;

		const float now = Time::Now();
		if (m_cooldown > 0.0f && (now - m_lastSpawnTime) < m_cooldown) return false;

		m_lastSpawnTime = now;
		return true;
	}

private:
	Key m_key = Key::Unknown;
	float m_cooldown = 0.0f;
	float m_lastSpawnTime = -9999.0f;
};

class LonerWaveSpawnerBehaviour : public KeyedSpawnerBase {
public:
	LonerWaveSpawnerBehaviour() { SetComponentName("LonerWaveSpawnerBehaviour"); }

	void SetCount(int count) { m_count = std::max(1, count); }
	void SetSpacing(float spacing) { m_spacing = spacing; }
	void SetForwardDistance(float d) { m_forward = d; }

protected:
	void Update() override {
		if (!ConsumeSpawnPress()) return;

		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;

		Vector2f shipPos, shipUp, shipRight;
		float shipRot = 0.0f;
		if (!GetShipPose(shipPos, shipUp, shipRight, shipRot)) return;

		const Vector2f base = shipPos + shipUp * m_forward;

		for (int i = 0; i < m_count; ++i) {
			const float off = (float)i - (float)(m_count - 1) * 0.5f;
			auto e = scene->CreateGameObject<Loner>("Loner");
			e->GetTransform()->SetPosition(base + shipRight * (off * m_spacing));
			e->GetTransform()->SetRotation(-90.0f);
		}
	}

private:
	int m_count = 1;
	float m_spacing = 96.0f;
	float m_forward = 520.0f;
};


class RusherWaveSpawnerBehaviour : public KeyedSpawnerBase {
public:
	RusherWaveSpawnerBehaviour() { SetComponentName("RusherWaveSpawnerBehaviour"); }

	void SetCount(int count) { m_count = std::max(1, count); }
	void SetSpacing(float spacing) { m_spacing = spacing; }
	void SetForwardDistance(float d) { m_forward = d; }

protected:
	void Update() override {
		if (!ConsumeSpawnPress()) return;

		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;

		Vector2f shipPos, shipUp, shipRight;
		float shipRot = 0.0f;
		if (!GetShipPose(shipPos, shipUp, shipRight, shipRot)) return;

		const Vector2f base = shipPos + shipUp * m_forward;
		for (int i = 0; i < m_count; ++i) {
			const float off = (float)i - (float)(m_count - 1) * 0.5f;
			auto e = scene->CreateGameObject<Rusher>("Rusher");
			e->GetTransform()->SetPosition(base + shipRight * (off * m_spacing));
		}
	}

private:
	int m_count = 6;
	float m_spacing = 48.0f;
	float m_forward = 520.0f;
};


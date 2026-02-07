// Xenon/EnemySpawners.hpp
#pragma once

#include "../GameEngine/GameEngine.h"

#include "Loner.hpp"
#include "Rusher.hpp"
#include "Drone.hpp"
#include "StoneAsteroids.hpp"
#include "MetalAsteroids.hpp"
#include "Pickups.hpp"

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

class DronePackSpawnerBehaviour : public KeyedSpawnerBase {
public:
	DronePackSpawnerBehaviour() { SetComponentName("DronePackSpawnerBehaviour"); }

	void SetPackSize(int count) { m_packSize = std::max(1, count); }
	void SetSpacing(float spacing) { m_spacing = spacing; }
	void SetForwardDistance(float d) { m_forward = d; }
	void SetForwardSpeed(float s) { m_forwardSpeed = s; }
	void SetSin(float amplitude, float hz) { m_sinAmplitude = amplitude; m_sinHz = hz; }

protected:
	void Update() override {
		if (!ConsumeSpawnPress()) return;

		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;

		Vector2f shipPos, shipUp, shipRight;
		float shipRot = 0.0f;
		if (!GetShipPose(shipPos, shipUp, shipRight, shipRot)) return;

		const Vector2f base = shipPos + shipUp * m_forward;

		auto cluster = std::make_shared<DroneClusterState>();
		cluster->killsSoFar = 0;
		cluster->basePoints = 5000;

		for (int i = 0; i < m_packSize; ++i) {
			auto d = scene->CreateGameObject<Drone>("Drone");

			d->GetTransform()->SetRotation(-90.0f);

			const Vector2f up = d->GetTransform()->GetUp();
			const float off = (float)i - (float)(m_packSize - 1) * 0.5f;
			d->GetTransform()->SetPosition(base + up * (off * m_spacing));

			auto b = d->GetComponent<DroneBehaviour>();
			b->SetCluster(cluster);
			b->SetForwardSpeed(m_forwardSpeed);
			b->SetSin(m_sinAmplitude, m_sinHz);
			b->SetPhase(0.6f * (float)i);
		}
	}

private:
	int m_packSize = 6;
	float m_spacing = 48.0f;
	float m_forward = 520.0f;

	float m_forwardSpeed = 140.0f;
	float m_sinAmplitude = 55.0f;
	float m_sinHz = 0.75f;
};


class StoneAsteroidSpawnerBehaviour : public KeyedSpawnerBase {
public:
	StoneAsteroidSpawnerBehaviour() { SetComponentName("StoneAsteroidSpawnerBehaviour"); }

	void SetSize(StoneAsteroidSize s) { m_size = s; }
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
			auto a = scene->CreateGameObject<StoneAsteroid>("StoneAsteroid", m_size);
			a->GetTransform()->SetPosition(base + shipRight * (off * m_spacing));
			a->GetTransform()->SetRotation(0.0f);
		}
	}

private:
	StoneAsteroidSize m_size = StoneAsteroidSize::Large96;
	int m_count = 1;
	float m_spacing = 96.0f;
	float m_forward = 520.0f;
};

class MetalAsteroidSpawnerBehaviour : public KeyedSpawnerBase {
public:
	MetalAsteroidSpawnerBehaviour() { SetComponentName("MetalAsteroidSpawnerBehaviour"); }

	void SetSize(MetalAsteroidSize s) { m_size = s; }
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
			auto a = scene->CreateGameObject<MetalAsteroid>("MetalAsteroid", m_size);
			a->GetTransform()->SetPosition(base + shipRight * (off * m_spacing));
			a->GetTransform()->SetRotation(0.0f);
		}
	}

private:
	MetalAsteroidSize m_size = MetalAsteroidSize::Large96;
	int m_count = 1;
	float m_spacing = 96.0f;
	float m_forward = 520.0f;
};

class PickupKeySpawnerBehaviour : public MonoBehaviour {
public:
	PickupKeySpawnerBehaviour() { SetComponentName("PickupKeySpawnerBehaviour"); }

	void SetHealKey(Key k) { m_keyHeal = k; }
	void SetWeaponKey(Key k) { m_keyWeapon = k; }
	void SetCompanionKey(Key k) { m_keyCompanion = k; }
	void SetForwardDistance(float d) { m_forward = d; }

protected:
	void Update() override {
		if (m_keyHeal != Key::Unknown && IsKeyPressed(m_keyHeal)) SpawnHeal();
		if (m_keyWeapon != Key::Unknown && IsKeyPressed(m_keyWeapon)) SpawnWeapon();
		if (m_keyCompanion != Key::Unknown && IsKeyPressed(m_keyCompanion)) SpawnCompanion();
	}

private:
	Vector2f SpawnPos() const {
		Vector2f shipPos, shipUp, shipRight;
		float shipRot = 0.0f;
		if (GetShipPose(shipPos, shipUp, shipRight, shipRot)) {
			return shipPos + shipUp * m_forward;
		}
		return Vector2f(520.0f, 0.0f);
	}

	void ApplySpawnTransform(GameObject* go) {
		if (!go) return;
		if (auto t = go->GetTransform()) {
			t->SetPosition(SpawnPos());
			t->SetRotation(-90.0f);
		}
	}

	void SpawnHeal() {
		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;
		auto p = scene->CreateGameObject<HealPickup>("HealPickup");
		ApplySpawnTransform(p.get());
	}

	void SpawnWeapon() {
		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;
		auto p = scene->CreateGameObject<WeaponPickup>("WeaponPickup");
		ApplySpawnTransform(p.get());
	}

	void SpawnCompanion() {
		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;
		auto p = scene->CreateGameObject<CompanionPickup>("CompanionPickup");
		ApplySpawnTransform(p.get());
	}

	Key m_keyHeal = Key::Unknown;
	Key m_keyWeapon = Key::Unknown;
	Key m_keyCompanion = Key::Unknown;

	float m_forward = 520.0f;
};


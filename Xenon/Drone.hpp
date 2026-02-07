// Xenon/Drone.hpp
#pragma once

#include "../GameEngine/GameEngine.h"

#include "EnemyEntity.hpp"
#include "LoopingSheet.hpp"
#include "ScorePopup.hpp"
#include "XenonGameMode.hpp"

#include <memory>
#include <cmath>
#include <algorithm>

// -----------------------------------------------------------------------------
// Drone enemy
// - Spawns in packs (shared cluster state -> escalating points)
// - Pack spawn is a PERFECT line along the drone's local UP (no randomness)
// - Movement is an S-curve relative to rotation:
//     * forward = -LocalUp
//     * side    = LocalRight (left/right)
// - Dies when it leaves the screen by the LEFT side only
// - Plays all frames in drone.bmp (colorkey 255,0,255)
// -----------------------------------------------------------------------------

struct DroneClusterState {
	int killsSoFar = 0;        // how many drones in this cluster have died
	int basePoints = 5000;     // first kill value
};

class DroneBehaviour : public EnemyEntity {
	BoxCollider2D* boxCol = nullptr;
	Animator* animator = nullptr;

	SpriteSheet* m_sheet = nullptr;
	AnimationClip m_clip{};
	AnimatorController m_ctrl{};

	// Movement params
	float m_forwardSpeed = 140.0f;      // along -LocalUp
	float m_sinAmplitude = 55.0f;       // along LocalRight
	float m_sinHz = 0.75f;
	float m_phase = 0.0f;

	// Pack line spawn (NO randomness)
	int   m_packIndex = 0;
	float m_packSpacing = 48.0f;
	bool  m_usePackLine = false;

	// Motion origin
	Vector2f m_startPos{};
	float    m_birthTime = 0.0f;

	std::shared_ptr<DroneClusterState> m_cluster;

protected:
	void Awake() override {
		EnemyEntity::Awake();

		// Kinematic, no gravity.
		if (rigidbody) {
			rigidbody->SetBodyType(Rigidbody2D::BodyType::Kinematic);
			rigidbody->SetGravityScale(0.0f);
			rigidbody->SetFixedRotation(true);
		}

		animator = GetComponent<Animator>().get();
		if (!animator) {
			THROW_ENGINE_EXCEPTION("Drone is missing Animator component");
		}

		m_sheet = LoadSpriteSheet("sheet.enemy.drone", "drone.bmp", Vector2i(32, 32), Vector3i(255, 0, 255));
		if (!m_sheet || !m_sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load drone spritesheet (drone.bmp)");
		}

		sprite->SetTexture(m_sheet->texture);
		sprite->SetFrameSize(m_sheet->frameSize);
		sprite->SetFrameIndex(0);
		sprite->SetLayerOrder(-2);

		BuildLoopAllFrames(m_sheet, 12.0f, m_clip, m_ctrl);
		animator->SetController(&m_ctrl);
		animator->Play("Loop", true);

		boxCol = dynamic_cast<BoxCollider2D*>(collider);
		if (boxCol) {
			boxCol->SetSize(sprite->GetFrameSize());
			boxCol->SetTrigger(true);
			boxCol->SetShouldSensorEvent(true);
		}
		else {
			THROW_ENGINE_EXCEPTION("Drone is missing BoxCollider2D component");
		}

		// Index 0 is the "base"; higher indices spawn "above" along local UP.
		transform->SetRotation(-90.f);
		if (m_usePackLine && transform) {
			const Vector2f up = transform->GetUp();
			transform->SetPosition(transform->GetPosition() + up * (m_packSpacing * (float)m_packIndex));

			m_phase = 0.6f * (float)m_packIndex;
		}

		m_startPos = transform ? transform->GetWorldPosition() : Vector2f{};
		m_birthTime = Time::Now();
	}

	void Update() override {
		EnemyEntity::Update();

		if (!transform) return;

		const float t = Time::Now() - m_birthTime;
		const float w = 6.283185307179586f * m_sinHz; // 2*pi*f

		// Relative axes
		const Vector2f up = transform->GetUp();
		const Vector2f right = transform->GetRight();

		// S path (relative to rotation):
		// forward: -up * speed
		// side:    right * amplitude * sin(...)
		const float side = m_sinAmplitude * std::sin(w * t + m_phase);
		const Vector2f p = m_startPos + (-up * (m_forwardSpeed * t)) + (right * side);

		transform->SetPosition(p);

		// Die ONLY when leaving by the LEFT side.
		// Keep these bounds aligned with the rest of your game.
		const Vector2f wp = transform->GetWorldPosition();
		if (wp.x < -420.0f) {
			Object::Destroy(GetGameObject());
		}
	}

	void OnDeath(GameObject* instigator) override {
		// Cluster-based scoring.
		int points = 5000;
		if (m_cluster) {
			points = m_cluster->basePoints * (m_cluster->killsSoFar + 1);
			m_cluster->killsSoFar += 1;
		}

		// Award points.
		if (auto* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr) {
			if (auto* mode = dynamic_cast<XenonGameMode*>(scene->GetGameMode())) {
				mode->AddScore(points);
			}
		}

		// Popup.
		if (auto* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr) {
			auto popup = scene->CreateGameObject<ScorePopup>("ScorePopup", points);
			popup->GetTransform()->SetPosition(GetTransform()->GetPosition());
		}

		Object::Destroy(GetGameObject());
	}

public:
	// Pack / scoring
	void SetCluster(const std::shared_ptr<DroneClusterState>& cluster) { m_cluster = cluster; }

	void SetPackLine(int indexInPack, float spacing) {
		m_usePackLine = true;
		m_packIndex = std::max(0, indexInPack);
		m_packSpacing = std::max(0.0f, spacing);
	}

	// Movement tuning
	void SetForwardSpeed(float s) { m_forwardSpeed = s; }
	void SetSin(float amplitude, float hz) {
		m_sinAmplitude = amplitude;
		m_sinHz = std::max(0.01f, hz);
	}

	void SetPhase(float phase) { m_phase = phase; }
};

class Drone : public GameObject {
public:
	explicit Drone(const std::string& name = "Drone")
		: GameObject(name) {
		AddComponent<Rigidbody2D>();
		AddComponent<SpriteRenderer>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<DroneBehaviour>();
	}
};

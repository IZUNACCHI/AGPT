#pragma once

#include "../GameEngine/GameEngine.h"
#include "../GameEngine/Animator.h"

#include "EnemyEntity.hpp"
#include "LoopingSheet.hpp"
#include "VFX.hpp"

#include <cstdlib>
#include <ctime>

// -----------------------------------------------------------------------------
// Stone asteroids (destructible)
// - Large (SAster96.bmp) splits -> 3 Medium (SAster64.bmp)
// - Medium (SAster64.bmp) splits -> 3 Small  (SAster32.bmp)
// - Small (SAster32.bmp) explodes on death
// - Moves "horizontally" relative to its rotation (so rotation changes travel direction)
// - Plays all frames in its spritesheet (colorkey 255,0,255)
// -----------------------------------------------------------------------------

enum class StoneAsteroidSize {
	Large96,
	Medium64,
	Small32,
};

inline Vector2i StoneAsteroidFrameSize(StoneAsteroidSize s) {
	switch (s) {
	case StoneAsteroidSize::Large96: return Vector2i(96, 96);
	case StoneAsteroidSize::Medium64: return Vector2i(64, 64);
	case StoneAsteroidSize::Small32: return Vector2i(32, 32);
	default: return Vector2i(64, 64);
	}
}

inline const char* StoneAsteroidSheetPath(StoneAsteroidSize s) {
	switch (s) {
	case StoneAsteroidSize::Large96: return "SAster96.bmp";
	case StoneAsteroidSize::Medium64: return "SAster64.bmp";
	case StoneAsteroidSize::Small32: return "SAster32.bmp";
	default: return "SAster64.bmp";
	}
}

inline int StoneAsteroidHealth(StoneAsteroidSize s) {
	switch (s) {
	case StoneAsteroidSize::Large96: return 6;
	case StoneAsteroidSize::Medium64: return 3;
	case StoneAsteroidSize::Small32: return 1;
	default: return 3;
	}
}

inline StoneAsteroidSize StoneAsteroidNext(StoneAsteroidSize s) {
	switch (s) {
	case StoneAsteroidSize::Large96: return StoneAsteroidSize::Medium64;
	case StoneAsteroidSize::Medium64: return StoneAsteroidSize::Small32;
	case StoneAsteroidSize::Small32: return StoneAsteroidSize::Small32;
	default: return StoneAsteroidSize::Small32;
	}
}

inline float Rand01_StoneAst() {
	static bool seeded = false;
	if (!seeded) {
		seeded = true;
		std::srand(static_cast<unsigned int>(std::time(nullptr)));
	}
	return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

inline float RandRange_StoneAst(float a, float b) {
	return a + (b - a) * Rand01_StoneAst();
}

class StoneAsteroidBehaviour : public EnemyEntity {
public:
	explicit StoneAsteroidBehaviour(StoneAsteroidSize size)
		: EnemyEntity(), m_size(size) {
		SetComponentName("StoneAsteroidBehaviour");
		m_maxHealth = StoneAsteroidHealth(size);
		m_health = m_maxHealth;
		m_points = 1000;
		m_damageOnContact = 1;
	}

protected:
	void Awake() override {
		EnemyEntity::Awake();

		if (rigidbody) {
			rigidbody->SetBodyType(Rigidbody2D::BodyType::Kinematic);
			rigidbody->SetGravityScale(0.0f);
			rigidbody->SetFixedRotation(true);
		}

		animator = GetComponent<Animator>().get();
		if (!animator) {
			THROW_ENGINE_EXCEPTION("StoneAsteroid is missing Animator component");
		}

		const Vector2i fs = StoneAsteroidFrameSize(m_size);
		const std::string key = std::string("sheet.hazard.stone.") + StoneAsteroidSheetPath(m_size);
		sheet = LoadSpriteSheet(key, StoneAsteroidSheetPath(m_size), fs, Vector3i(255, 0, 255));
		if (!sheet || !sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load stone asteroid spritesheet");
		}

		sprite->SetTexture(sheet->texture);
		sprite->SetFrameSize(fs);
		sprite->SetFrameIndex(0);
		sprite->SetLayerOrder(-2);

		BuildLoopAllFrames(sheet, 12.0f, clip, ctrl);
		animator->SetController(&ctrl);
		animator->Play("Loop", true);

		box = dynamic_cast<BoxCollider2D*>(collider);
		if (!box) {
			THROW_ENGINE_EXCEPTION("StoneAsteroid is missing BoxCollider2D component");
		}
		box->SetSize(sprite->GetFrameSize());
		box->SetTrigger(true);
		box->SetShouldSensorEvent(true);

		// Give it a slight random rotation if not set by spawner.
		if (transform && std::abs(transform->GetWorldRotation()) < 0.001f) {
			transform->SetRotation(RandRange_StoneAst(-25.0f, 25.0f));
		}
	}

	void Update() override {
		EnemyEntity::Update();

		if (rigidbody && transform) {
			const Vector2f dir = -transform->GetRight();
			rigidbody->SetLinearVelocity(dir * m_speed);
		}

		if (transform) {
			const Vector2f p = transform->GetWorldPosition();
			if (p.x < -460.0f || p.y < -380.0f || p.y > 380.0f) {
				Object::Destroy(GetGameObject());
			}
		}
	}

	void OnDeath(GameObject* instigator) override {
		// Explosion.
		if (auto* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr) {
			auto vfx = scene->CreateGameObject<ExplosionVFX>("ExplosionVFX");
			vfx->GetTransform()->SetPosition(GetTransform()->GetPosition());
		}

		// Split if large/medium.
		if (m_size != StoneAsteroidSize::Small32) {
			SplitIntoThree();
		}

		// Award points and popup.
		EnemyEntity::OnDeath(instigator);
	}

private:
	void SplitIntoThree();

	StoneAsteroidSize m_size = StoneAsteroidSize::Medium64;
	float m_speed = 110.0f;

	BoxCollider2D* box = nullptr;
	Animator* animator = nullptr;
	SpriteSheet* sheet = nullptr;
	AnimationClip clip{};
	AnimatorController ctrl{};
};

class StoneAsteroid : public GameObject {
public:
	explicit StoneAsteroid(const std::string& name = "StoneAsteroid", StoneAsteroidSize size = StoneAsteroidSize::Large96)
		: GameObject(name) {
		AddComponent<Rigidbody2D>();
		AddComponent<SpriteRenderer>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<StoneAsteroidBehaviour>(size);
	}
};

// --- Split implementation (defined after StoneAsteroid is complete to avoid template issues) ---
inline void StoneAsteroidBehaviour::SplitIntoThree() {
	Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
	if (!scene || !transform) return;

	const StoneAsteroidSize childSize = StoneAsteroidNext(m_size);
	const Vector2f basePos = transform->GetWorldPosition();
	const float baseRot = transform->GetWorldRotation();

	// Spread in a small fan.
	const float angles[3] = { -18.0f, 0.0f, 18.0f };
	const Vector2f offsets[3] = {
		Vector2f(-10.0f, -10.0f),
		Vector2f(0.0f, 0.0f),
		Vector2f(-10.0f, 10.0f)
	};

	for (int i = 0; i < 3; ++i) {
		auto child = scene->CreateGameObject<StoneAsteroid>("StoneAsteroid", childSize);
		child->GetTransform()->SetPosition(basePos + offsets[i]);
		child->GetTransform()->SetRotation(baseRot + angles[i]);
	}
}

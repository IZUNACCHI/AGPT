#pragma once

#include "../GameEngine/GameEngine.h"
#include "../GameEngine/Animator.h"

#include "EnemyEntity.hpp"
#include "LoopingSheet.hpp"

#include <cstdlib>
#include <ctime>

// -----------------------------------------------------------------------------
// Metal asteroids (indestructible hazards)
// - Three sizes with sprite sheets (names vary a bit between assets):
//     MAster96.bmp / Master96.bmp
//     MAster64.bmp / Master64.bmp
//     MAster32.bmp / Master32.bmp
// - Play the full sprite sheet, colorkey 255,0,255
// - Ignore damage, but still damage player/companion on contact (EnemyEntity)
// -----------------------------------------------------------------------------

enum class MetalAsteroidSize {
	Large96,
	Medium64,
	Small32,
};

inline Vector2i MetalAsteroidFrameSize(MetalAsteroidSize s) {
	switch (s) {
	case MetalAsteroidSize::Large96: return Vector2i(96, 96);
	case MetalAsteroidSize::Medium64: return Vector2i(64, 64);
	case MetalAsteroidSize::Small32: return Vector2i(32, 32);
	default: return Vector2i(64, 64);
	}
}

inline const char* MetalAsteroidSheetPathPrimary(MetalAsteroidSize s) {
	switch (s) {
	case MetalAsteroidSize::Large96: return "MAster96.bmp";
	case MetalAsteroidSize::Medium64: return "MAster64.bmp";
	case MetalAsteroidSize::Small32: return "MAster32.bmp";
	default: return "MAster64.bmp";
	}
}

inline const char* MetalAsteroidSheetPathAlt(MetalAsteroidSize s) {
	switch (s) {
	case MetalAsteroidSize::Large96: return "Master96.bmp";
	case MetalAsteroidSize::Medium64: return "Master64.bmp";
	case MetalAsteroidSize::Small32: return "Master32.bmp";
	default: return "Master64.bmp";
	}
}

inline float Rand01_MetalAst() {
	static bool seeded = false;
	if (!seeded) {
		seeded = true;
		std::srand(static_cast<unsigned int>(std::time(nullptr)) ^ 0xA531u);
	}
	return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

inline float RandRange_MetalAst(float a, float b) {
	return a + (b - a) * Rand01_MetalAst();
}

class MetalAsteroidBehaviour : public EnemyEntity {
public:
	explicit MetalAsteroidBehaviour(MetalAsteroidSize size)
		: EnemyEntity(), m_size(size) {
		SetComponentName("MetalAsteroidBehaviour");
		m_points = 0; // indestructible
		m_damageOnContact = 1;
		m_maxHealth = 999999;
		m_health = m_maxHealth;
	}

	// Indestructible: ignore damage.
	void ApplyDamage(int amount, GameObject* instigator = nullptr) override {
		(void)amount;
		(void)instigator;
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
			THROW_ENGINE_EXCEPTION("MetalAsteroid is missing Animator component");
		}

		const Vector2i fs = MetalAsteroidFrameSize(m_size);

		// Try primary filename, then fallback.
		{
			const char* p1 = MetalAsteroidSheetPathPrimary(m_size);
			const std::string key1 = std::string("sheet.hazard.metal.") + p1;
			sheet = LoadSpriteSheet(key1, p1, fs, Vector3i(255, 0, 255));
		}
		if (!sheet || !sheet->IsValid()) {
			const char* p2 = MetalAsteroidSheetPathAlt(m_size);
			const std::string key2 = std::string("sheet.hazard.metal.") + p2;
			sheet = LoadSpriteSheet(key2, p2, fs, Vector3i(255, 0, 255));
		}
		if (!sheet || !sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load metal asteroid spritesheet");
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
			THROW_ENGINE_EXCEPTION("MetalAsteroid is missing BoxCollider2D component");
		}
		box->SetSize(sprite->GetFrameSize());
		box->SetTrigger(true);
		box->SetShouldSensorEvent(true);

		if (transform && std::abs(transform->GetWorldRotation()) < 0.001f) {
			transform->SetRotation(RandRange_MetalAst(-25.0f, 25.0f));
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

private:
	MetalAsteroidSize m_size = MetalAsteroidSize::Medium64;
	float m_speed = 105.0f;

	BoxCollider2D* box = nullptr;
	Animator* animator = nullptr;
	SpriteSheet* sheet = nullptr;
	AnimationClip clip{};
	AnimatorController ctrl{};
};

class MetalAsteroid : public GameObject {
public:
	explicit MetalAsteroid(const std::string& name = "MetalAsteroid", MetalAsteroidSize size = MetalAsteroidSize::Large96)
		: GameObject(name) {
		AddComponent<Rigidbody2D>();
		AddComponent<SpriteRenderer>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<MetalAsteroidBehaviour>(size);
	}
};

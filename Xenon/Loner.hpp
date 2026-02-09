#pragma once

#include <GameEngine/GameEngine.h>
#include "XenonViewportComponents.hpp"
#include <GameEngine/Animator.h>

#include "EnemyEntity.hpp"
#include "Projectile.hpp"
#include "LoopingSheet.hpp"

class LonerBehaviour : public EnemyEntity {
	BoxCollider2D* boxCol = nullptr;
	EnemyProjectileLauncher* launcher = nullptr;
	Animator* animator = nullptr;

	SpriteSheet* m_sheet = nullptr;
	AnimationClip m_clip{};
	AnimatorController m_ctrl{};

	float m_speed = 120.0f;
	float m_dir = 1.0f;

protected:
	void Awake() override {
		EnemyEntity::Awake();

		if (rigidbody) {
			rigidbody->SetBodyType(Rigidbody2D::BodyType::Kinematic);
			rigidbody->SetGravityScale(0.0f);
			rigidbody->SetFixedRotation(true);
		}

		launcher = GetComponent<EnemyProjectileLauncher>().get();
		if (launcher) {
			launcher->SetCooldown(2.0f);
			launcher->SetProjectileSpeed(500.0f);
			launcher->SetDamage(1);
			launcher->SetMuzzleOffset(Vector2f(0.0f, -34.0f));
		}

		animator = GetComponent<Animator>().get();
		if (!animator) {
			THROW_ENGINE_EXCEPTION("Loner is missing Animator component");
		}

		m_sheet = LoadSpriteSheet("sheet.enemy.loner", "LonerA.bmp", Vector2i(64, 64), Vector3i(255, 0, 255));
		if (!m_sheet || !m_sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load Loner spritesheet (LonerA.bmp)");
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
			THROW_ENGINE_EXCEPTION("Loner is missing BoxCollider2D component");
		}

		if (transform) {
			transform->SetRotation(-90.0f);
		}

		// Set initial movement once; BounceOffViewport2D will reflect velocity on impact.
		if (rigidbody && transform) {
			const Vector2f dir = transform->GetRight() * m_dir;
			rigidbody->SetLinearVelocity(dir * m_speed);
		}
	}

	void Start() override {
		InvokeRepeating([this]() {
			if (!launcher) return;
			auto ship = Scene::FindGameObject("SpaceShip");
			if (ship && ship->IsActiveInHierarchy()) {
				launcher->TryFireToward(ship->GetTransform()->GetWorldPosition());
			}
			}, 1.0f, 2.0f, MonoBehaviour::InvokeTickPolicy::WhileBehaviourEnabled);
	}

	void Update() override {
		EnemyEntity::Update();
		// Movement is handled by Rigidbody2D velocity + BounceOffViewport2D.
	}

};

class Loner : public GameObject {
public:
	explicit Loner(const std::string& name = "Loner")
		: GameObject(name) {
		AddComponent<Rigidbody2D>();
		AddComponent<SpriteRenderer>();
		AddComponent<BounceOffViewport2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<EnemyProjectileLauncher>();
		AddComponent<Animator>();
		AddComponent<LonerBehaviour>();
	}
};

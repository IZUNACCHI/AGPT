#pragma once

#include <GameEngine/GameEngine.h>
#include "XenonViewportComponents.hpp"
#include <GameEngine/Animator.h>

#include "EnemyEntity.hpp"
#include "XenonAssetKeys.h"
#include "LoopingSheet.hpp"

class RusherBehaviour : public EnemyEntity {
	BoxCollider2D* boxCol = nullptr;
	Animator* animator = nullptr;

	SpriteSheet* m_sheet = nullptr;
	AnimationClip m_clip{};
	AnimatorController m_ctrl{};

	float m_speed = 160.0f;

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
			THROW_ENGINE_EXCEPTION("Rusher is missing Animator component");
		}

		m_sheet = LoadSpriteSheet(
			XenonAssetKeys::Sheets::Rusher,
			XenonAssetKeys::Files::RusherBmp,
			Vector2i(64, 32),
			Vector3i(255, 0, 255)
		);

		if (!m_sheet || !m_sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load rusher spritesheet (rusher.bmp)");
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
			THROW_ENGINE_EXCEPTION("Rusher is missing BoxCollider2D component");
		}

		if (transform) {
			transform->SetRotation(-90.0f);
		}
	}

	void Update() override {
		EnemyEntity::Update();

		if (rigidbody && transform) {
			const Vector2f dir = -transform->GetUp();
			rigidbody->SetLinearVelocity(dir * m_speed);
		}

		// Offscreen despawn is handled by DespawnOffscreen2D.
	}
};

class Rusher : public GameObject {
public:
	explicit Rusher(const std::string& name = "Rusher")
		: GameObject(name) {
		AddComponent<Rigidbody2D>();
		AddComponent<SpriteRenderer>();
		AddComponent<XenonDespawnOffscreen2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<RusherBehaviour>();
	}
};

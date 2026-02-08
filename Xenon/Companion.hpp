#pragma once

#include "AllyEntity.hpp"
#include "Projectile.hpp"

#include "XenonAssetKeys.h"

#include <GameEngine/GameEngine.h>

#include <algorithm>
#include <cmath>

// -----------------------------------------------------------------------------
// Companion
// - Up to two companions can follow the Spaceship.
// - They have their own health (5 hits) and their own missile tier.
// - They fire when the player fires.
// - Visuals: clone.bmp, loop EVERYTHING but the last line.
// -----------------------------------------------------------------------------

namespace CompanionAnim {
	inline SpriteSheet* GetSheet() {
		// Let AssetManager own the cache. No header-local static pointers.
		return LoadSpriteSheet(XenonAssetKeys::Sheets::CompanionClone, "clone.bmp", Vector2i(32, 32), Vector3i(255, 0, 255));
	}

	// Controller/clip are templates shared by all companions. Runtime state is in
	// the Animator component instance.
	inline bool g_built = false;
	inline AnimationClip g_clip{};
	inline AnimatorController g_ctrl{};

	inline void BuildOnce() {
		if (g_built) return;
			SpriteSheet* sheet = GetSheet();
			if (!sheet || !sheet->IsValid()) {
				THROW_ENGINE_EXCEPTION("Failed to load companion spritesheet (clone.bmp)");
			}

			const Vector2i texSize = sheet->texture->GetSize();
			const int cols = sheet->frameSize.x > 0 ? (texSize.x / sheet->frameSize.x) : 0;
			const int rows = sheet->frameSize.y > 0 ? (texSize.y / sheet->frameSize.y) : 0;
			if (cols <= 0 || rows <= 0) {
				THROW_ENGINE_EXCEPTION("clone.bmp has invalid frame grid");
			}

			// Loop everything but the last row.
			const int loopRows = std::max(0, rows - 1);
			const int frameCount = cols * loopRows;
			if (frameCount <= 0) {
				THROW_ENGINE_EXCEPTION("clone.bmp has no frames to loop (rows-1 == 0?)");
			}

			g_clip = AnimationClip{};
			g_clip.name = "Loop";
			g_clip.sheet = sheet;
			g_clip.fps = 16.0f;
			g_clip.loop = true;
			g_clip.frames.clear();
			g_clip.frames.reserve((size_t)frameCount);
			for (int i = 0; i < frameCount; ++i) {
				g_clip.frames.push_back(i);
			}

			g_ctrl = AnimatorController{};
			AnimState st;
			st.id = 0;
			st.name = "Loop";
			st.clip = &g_clip;
			g_ctrl.states = { st };
			g_ctrl.entryState = 0;
			g_ctrl.transitions.clear();
			g_ctrl.parameters.clear();

			g_built = true;
		}

	inline AnimatorController* GetController(AnimationClip*& outClipPtr) {
		BuildOnce();
		outClipPtr = &g_clip;
		return &g_ctrl;
	}
}

class CompanionBehaviour : public AllyEntity {
public:
	CompanionBehaviour()
		: AllyEntity() {
		SetComponentName("CompanionBehaviour");
	}

	void BindToShip(GameObject* ship, const Vector2f& localOffset) {
		m_ship = ship;
		m_localOffset = localOffset;
	}

	// Called by the Spaceship when it fires.
	void TryFire() {
		if (!m_launcher) return;
		m_launcher->TryFireForward();
	}

protected:
	void Awake() override {
		AllyEntity::Awake();

		// Companion has 5 HP ("dies if damaged 5 times").
		SetMaxHealth(5);

		// Visuals.
		SpriteSheet* sheet = CompanionAnim::GetSheet();
		if (!sheet || !sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load clone.bmp spritesheet");
		}
		sprite->SetTexture(sheet->texture);
		sprite->SetFrameSize(sheet->frameSize);
		sprite->SetFrameIndex(0);
		sprite->SetLayerOrder(1);

		// Kinematic: we manually place it relative to the ship.
		rigidbody->SetBodyType(Rigidbody2D::BodyType::Kinematic);
		rigidbody->SetFixedRotation(true);
		rigidbody->SetIsBullet(true);

		// Shooter.
		m_launcher = GetComponent<PlayerProjectileLauncher>().get();
		if (m_launcher) {
			m_launcher->SetCooldown(0.15f);
			m_launcher->SetProjectileSpeed(900.0f);
			m_launcher->SetDamage(0);
			m_launcher->SetMissileType(MissileType::Light);
			m_launcher->SetMuzzleOffset(Vector2f(0.0f, 34.0f));
		}

		// Animator graph.
		Animator* animator = GetComponent<Animator>().get();
		if (!animator) {
			THROW_ENGINE_EXCEPTION("Companion is missing Animator component");
		}
		AnimationClip* clip = nullptr;
		AnimatorController* ctrl = CompanionAnim::GetController(clip);
		if (!ctrl || !clip) {
			THROW_ENGINE_EXCEPTION("Failed to build companion animator controller");
		}
		animator->SetController(ctrl);
		animator->Play("Loop", true);
	}

	void LateUpdate() override {
		// Follow the ship (if it exists).
		if (!m_ship) return;
		if (m_ship->IsMarkedForDestruction() || m_ship->IsDestroyed()) {
			Object::Destroy(GetGameObject());
			return;
		}
		Transform* shipT = m_ship->GetTransform();
		if (!shipT) return;

		// Position = ship local point transformed to world.
		const Vector2f worldPos = shipT->GetWorldMatrix() * m_localOffset;
		// Companions copy the ship rotation so their forward matches.
		const float shipRot = shipT->GetWorldRotation();

		if (rigidbody) {
			rigidbody->SetPosition(worldPos);
			rigidbody->SetRotation(shipRot);
		}
		else {
			GetTransform()->SetPosition(worldPos);
			GetTransform()->SetRotation(shipRot);
		}
	}

private:
	GameObject* m_ship = nullptr;
	Vector2f m_localOffset = Vector2f::Zero();
	PlayerProjectileLauncher* m_launcher = nullptr;
};

class Companion : public GameObject {
public:
	explicit Companion(const std::string& name = "Companion")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<Rigidbody2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<PlayerProjectileLauncher>();
		AddComponent<CompanionBehaviour>();
	}

	void BindToShip(GameObject* ship, const Vector2f& localOffset) {
		auto b = GetComponent<CompanionBehaviour>();
		if (b) b->BindToShip(ship, localOffset);
	}

	void TryFire() {
		auto b = GetComponent<CompanionBehaviour>();
		if (b) b->TryFire();
	}
};

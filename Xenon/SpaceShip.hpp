#pragma once
#include <GameEngine/GameEngine.h>
#include <GameEngine/ViewportUtils.h>
#include <GameEngine/Animator.h>
#include "AllyEntity.hpp"
#include "Projectile.hpp"
#include "Companion.hpp"
#include "XenonAssetKeys.h"
#include "XenonViewportComponents.hpp"
#include <memory>
#include <algorithm>
#include <cmath>



namespace ShipAnim {
	inline SpriteSheet* GetSheet() {
		// Let AssetManager own the cache. This call is cheap after the first time.
		return LoadSpriteSheet(XenonAssetKeys::Sheets::Ship2, XenonAssetKeys::Files::Ship2Bmp, Vector2i(64, 64), Vector3i(255, 0, 255));
	}

	struct Built {
		AnimatorController* controller = nullptr;
		AnimationClip* normalTurn = nullptr;
		AnimationClip* invulnTurn = nullptr;
		AnimationClip* death = nullptr;
	};

	// NOTE: We keep templates in this header (ship-only), but avoid function-local statics
	// in inline functions. We use C++17 inline variables instead, which are singletons
	// across translation units.
	inline bool g_built = false;
	inline AnimationClip g_clipNormal{};
	inline AnimationClip g_clipInvuln{};
	inline AnimationClip g_clipDeath{};
	inline AnimatorController g_ctrl{};
	inline Built g_out{};

	inline void BuildOnce() {
		if (g_built) return;

		SpriteSheet* sheet = GetSheet();
		if (!sheet || !sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load Ship2.bmp spritesheet");
		}

			const Vector2i texSize = sheet->texture->GetSize();
			const int cols = sheet->frameSize.x > 0 ? (texSize.x / sheet->frameSize.x) : 0;
			const int rows = sheet->frameSize.y > 0 ? (texSize.y / sheet->frameSize.y) : 0;
			if (cols <= 0 || rows < 3) {
				THROW_ENGINE_EXCEPTION("Ship2.bmp grid invalid (expected at least 3 rows)");
			}
			if (cols < 7) {
				THROW_ENGINE_EXCEPTION("Ship2.bmp grid invalid (expected at least 7 columns)");
			}

		auto buildRowClip = [&](AnimationClip& clip, const std::string& name, int row, float fps, bool loop) {
			clip = AnimationClip{};
			clip.name = name;
			clip.sheet = sheet;
			clip.fps = fps;
			clip.loop = loop;
			clip.frames.clear();
			clip.frames.reserve(7);
			for (int c = 0; c < 7; ++c) {
				clip.frames.push_back(row * cols + c);
			}
		};

			// Turning rows: non-looping so SeekNormalized doesn't wrap.
		buildRowClip(g_clipNormal, "NormalTurn", 0, 14.0f, false);
		buildRowClip(g_clipInvuln, "InvulnTurn", 1, 14.0f, false);
			// Death: play once.
		buildRowClip(g_clipDeath, "Death", 2, 12.0f, false);

		g_ctrl = AnimatorController{};
		g_ctrl.parameters = {
				AnimParamDef{ "Invuln", AnimParamType::Bool, 0.0f, 0, false },
				AnimParamDef{ "Die", AnimParamType::Trigger, 0.0f, 0, false },
			};

		AnimState st0; st0.id = 0; st0.name = "Normal"; st0.clip = &g_clipNormal;
		AnimState st1; st1.id = 1; st1.name = "Invuln"; st1.clip = &g_clipInvuln;
		AnimState st2; st2.id = 2; st2.name = "Death"; st2.clip = &g_clipDeath;
		g_ctrl.states = { st0, st1, st2 };
		g_ctrl.entryState = 0;

		g_ctrl.transitions.clear();
			// Any state -> Death
			{
				AnimTransition tr;
				tr.fromState = -1;
				tr.toState = 2;
				tr.hasExitTime = false;
				tr.conditions = { AnimCondition{ "Die", AnimCondOp::TriggerSet } };
		g_ctrl.transitions.push_back(tr);
			}
			// Normal -> Invuln
			{
				AnimTransition tr;
				tr.fromState = 0;
				tr.toState = 1;
				tr.conditions = { AnimCondition{ "Invuln", AnimCondOp::BoolTrue } };
		g_ctrl.transitions.push_back(tr);
			}
			// Invuln -> Normal
			{
				AnimTransition tr;
				tr.fromState = 1;
				tr.toState = 0;
				tr.conditions = { AnimCondition{ "Invuln", AnimCondOp::BoolFalse } };
		g_ctrl.transitions.push_back(tr);
			}

		g_out.controller = &g_ctrl;
		g_out.normalTurn = &g_clipNormal;
		g_out.invulnTurn = &g_clipInvuln;
		g_out.death = &g_clipDeath;
		g_built = true;
	}

	inline const Built& Get() {
		BuildOnce();
		return g_out;
	}
}



class SpaceShipBehaviour : public AllyEntity {
	
protected:
	Texture* shipTexture = nullptr;
	BoxCollider2D* boxCol = nullptr;
	PlayerProjectileLauncher* launcher = nullptr;
	Animator* m_animator = nullptr;
	std::shared_ptr<AudioSource> m_gunAudio;
	AudioClip* m_gunClip = nullptr;

	// Invulnerability (Ship2.bmp row 1)
	const float m_invulnDuration = 1.5f;
	bool m_isInvulnerable = false;
	MonoBehaviour::InvokeHandle m_invulnInvoke = 0;

	// Death animation
	bool m_isDying = false;
	float m_deathLength = 0.0f;
	MonoBehaviour::InvokeHandle m_deathInvoke = 0;

	// Up to two companions.
	std::weak_ptr<Companion> m_leftCompanion;
	std::weak_ptr<Companion> m_rightCompanion;

	const Vector2f moveSpeed = Vector2f(300, 280);

	// Local offsets relative to the ship.
	const Vector2f m_leftOffset = Vector2f(-60.0f, 0.0f);
	const Vector2f m_rightOffset = Vector2f(60.0f, 0.0f);

	// Screen clamping is handled by the engine-generic ClampToViewport2D component.

	// Spawn a companion if there is a free slot. Returns true if one was added.
	bool AddCompanion() {
		Scene* scene = GetGameObject()->GetScene();
		if (!scene) return false;

		// Prefer filling left slot first.
		if (m_leftCompanion.expired()) {
			auto c = scene->CreateGameObject<Companion>("CompanionLeft");
			c->BindToShip(GetGameObject(), m_leftOffset);
			m_leftCompanion = c;
			return true;
		}
		if (m_rightCompanion.expired()) {
			auto c = scene->CreateGameObject<Companion>("CompanionRight");
			c->BindToShip(GetGameObject(), m_rightOffset);
			m_rightCompanion = c;
			return true;
		}
		return false;
	}

	

void EndInvulnerability() {
	m_isInvulnerable = false;
	if (m_animator) m_animator->SetBool("Invuln", false);
}

void FinishDeath() {
	Object::Destroy(GetGameObject());
}
void KillCompanions() {
		if (auto l = m_leftCompanion.lock()) {
			Object::Destroy(l);
		}
		if (auto r = m_rightCompanion.lock()) {
			Object::Destroy(r);
		}
		m_leftCompanion.reset();
		m_rightCompanion.reset();
	}

	void Awake() override {
		AllyEntity::Awake();

		launcher = GetComponent<PlayerProjectileLauncher>().get();
		if (launcher) {
			launcher->SetCooldown(0.15f);
			launcher->SetProjectileSpeed(900.0f);
			// Let missile damage come from the missile type (Light/Medium/Heavy).
			// If you really want to override damage, call SetDamage(...) again.
			launcher->SetDamage(0);
			launcher->SetMissileType(MissileType::Light);
			// Gun position is "above" the ship in local space.
			// (Local-space offset rotates with the ship.)
			launcher->SetMuzzleOffset(Vector2f(0.0f, 34.0f));
		}

		// --- Visuals + Animator (Ship2.bmp) ---
		ShipAnim::Built anim = ShipAnim::Get();
		SpriteSheet* sheet = ShipAnim::GetSheet();
		if (!sheet || !sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load Ship2.bmp spritesheet");
		}
		sprite->SetTexture(sheet->texture);
		sprite->SetFrameSize(sheet->frameSize);
		sprite->SetFrameIndex(3); // center frame as a safe default

		m_animator = GetComponent<Animator>().get();
		if (!m_animator) {
			THROW_ENGINE_EXCEPTION("SpaceShip is missing Animator component");
		}
		m_animator->SetController(anim.controller);
		m_animator->Play("Normal", true);
		m_animator->SetBool("Invuln", false);
		// Try to start centered.
		m_animator->SeekNormalized(0.5f, 2.0f);
		m_deathLength = anim.death ? anim.death->GetLengthSeconds() : 0.0f;

		// --- Audio ---
		// Play gun.wav when the player fires.
		m_gunAudio = GetComponent<AudioSource>();
		m_gunClip = LoadAudioClip(XenonAssetKeys::Audio::GunWav);
		if (m_gunAudio && m_gunClip) {
			m_gunAudio->SetClip(m_gunClip);
			m_gunAudio->SetLoop(false);
			m_gunAudio->SetGain(1.0f);
		}


		rigidbody->SetBodyType(Rigidbody2D::BodyType::Dynamic);
		rigidbody->SetFixedRotation(true);
		rigidbody->SetIsBullet(true);

		boxCol = dynamic_cast<BoxCollider2D*>(collider);
		if (boxCol) {
			boxCol->SetSize(sprite->GetFrameSize());
			boxCol->SetTrigger(false);
			boxCol->SetShouldSensorEvent(true);
		}
		else {
			THROW_ENGINE_EXCEPTION("SpaceShip " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ")" + " is missing BoxCollider2D component");
		}

		SetMaxHealth(100);
		SetHealth(GetMaxHealth());

		// Xenon faces right: make local +Y (up) point to the right of the screen.
		GetTransform()->SetRotation(-90.0f);
	};

	void ApplyDamage(int amount, GameObject* instigator = nullptr) override {
		// Ignore damage while invulnerable.
		if (!m_alive) return;
		if (m_isDying) return;
		if (m_isInvulnerable) return;

		Entity::ApplyDamage(amount, instigator);

		// If we survived the hit, enter invulnerability.
		if (m_alive) {
			m_isInvulnerable = true;
			if (m_animator) m_animator->SetBool("Invuln", true);
			if (m_invulnInvoke) CancelInvoke(m_invulnInvoke);
			m_invulnInvoke = Invoke([this]() { EndInvulnerability(); }, m_invulnDuration, MonoBehaviour::InvokeTickPolicy::WhileBehaviourEnabled);
		}
	}

	void Update() override {
		// If dying, ignore input/movement.
		if (m_isDying) {
			return;
		}

		// Move the spaceship based on user input
		Vector2f velocity = Vector2f::Zero();
		if (IsKeyDown(Key::W) || Input::GetGamepadLeftStick().y > 0.0f) {
			velocity.y += 1.0f;
		}
		if (IsKeyDown(Key::S) || Input::GetGamepadLeftStick().y < 0.0f) {
			velocity.y -= 1.0f;
		}
		if (IsKeyDown(Key::A) || Input::GetGamepadLeftStick().x < 0.0f) {
			velocity.x -= 1.0f;
		}
		if (IsKeyDown(Key::D) || Input::GetGamepadLeftStick().x > 0.0f) {
			velocity.x += 1.0f;
		}
		if (velocity.LengthSquared() > 0.0f) {
			velocity = velocity.Normalized() * moveSpeed;
		}

		rigidbody->SetLinearVelocity(velocity);

		// Turning animation
		if (m_animator) {
			const std::string st = m_animator->GetCurrentStateName();
			if (st != "Death") {
				float targetN = 0.5f;

				// Turning is driven by UP/DOWN instead of LEFT/RIGHT.
				// (This only affects the visual banking animation; movement still uses WASD / stick X+Y.)
				const float stickY = Input::GetGamepadLeftStick().y;
				const bool up = IsKeyDown(Key::W) || stickY > 0.25f;
				const bool down = IsKeyDown(Key::S) || stickY < -0.25f;
				if (up && !down) targetN = 0.0f;          // bank left when moving up
				else if (down && !up) targetN = 1.0f;     // bank right when moving down
				m_animator->SeekNormalized(targetN, 2.0f);
			}
		}

		// Fire a projectile when Space is pressed.
		if (launcher && (IsKeyPressed(Key::Space) || IsGamepadButtonDown(GamepadButton::South))) {
			if (launcher->TryFireForward()) {
				if (m_gunAudio && m_gunClip) {
					m_gunAudio->Play();
				}
				// Companions fire when the player fires.
				if (auto l = m_leftCompanion.lock()) l->TryFire();
				if (auto r = m_rightCompanion.lock()) r->TryFire();
			}
		}
	}

	// LateUpdate is no longer needed here (ClampToViewport2D runs in LateUpdate).

	void OnDeath(GameObject* instigator) override {
		(void)instigator;
		if (m_isDying) return;
		m_isDying = true;
		m_isInvulnerable = false;
		if (m_invulnInvoke) CancelInvoke(m_invulnInvoke); m_invulnInvoke = 0;
		KillCompanions();

		// Stop movement.
		if (rigidbody) {
			rigidbody->SetLinearVelocity(Vector2f::Zero());
		}
		// Play death animation.
		if (m_animator) {
			m_animator->SetBool("Invuln", false);
			m_animator->SetTrigger("Die");
		}
		if (m_deathInvoke) CancelInvoke(m_deathInvoke);
		m_deathInvoke = Invoke([this]() { FinishDeath(); }, (m_deathLength > 0.0f ? m_deathLength : 0.01f), MonoBehaviour::InvokeTickPolicy::WhileBehaviourEnabled);
	}

public:
	// Exposed so CompanionPickup can grant a companion.
	bool TryAddCompanion() { return AddCompanion(); }

};

class SpaceShip : public GameObject {
public:
	explicit SpaceShip(const std::string& name = "SpaceShip")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<Rigidbody2D>();
		AddComponent<ClampToViewPort2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<AudioSource>();
		AddComponent<PlayerProjectileLauncher>();
		AddComponent<SpaceShipBehaviour>();
	}
};

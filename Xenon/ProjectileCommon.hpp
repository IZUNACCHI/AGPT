#pragma once

#include "../GameEngine/GameEngine.h"
#include "Faction.hpp"
#include "IDamageable.hpp"
#include "VFX.hpp"

// -----------------------------------------------------------------------------
// Projectile common code
// - Base projectile hit logic
// - Faction filtering
// -----------------------------------------------------------------------------

// Find any IDamageable implemented by a MonoBehaviour on a GameObject.
inline IDamageable* FindDamageable(GameObject* go) {
	if (!go) return nullptr;

	auto behaviours = go->GetComponents<MonoBehaviour>();
	for (const auto& b : behaviours) {
		if (!b) continue;
		if (auto* asDamageable = dynamic_cast<IDamageable*>(b.get())) {
			return asDamageable;
		}
	}
	return nullptr;
}

class ProjectileBehaviour : public MonoBehaviour {
public:
	explicit ProjectileBehaviour(const std::string& name = "ProjectileBehaviour") {
		SetComponentName(name);
	}

	void SetFaction(Faction faction) { m_faction = faction; }
	Faction GetFaction() const { return m_faction; }

	void SetDamage(int damage) { m_damage = damage; }
	int GetDamage() const { return m_damage; }

	void SetSpeed(float speed) { m_speed = speed; }
	float GetSpeed() const { return m_speed; }

	void SetLifetime(float seconds) { m_lifetimeSeconds = seconds; }
	float GetLifetime() const { return m_lifetimeSeconds; }

	void SetDirection(const Vector2f& dir) { m_direction = dir; }
	Vector2f GetDirection() const { return m_direction; }

protected:
	// Derived projectiles use this to set up initial visuals (texture / frame size)
	// before the collider is sized.
	virtual void ConfigureSprite() {}

	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		rigidbody = GetComponent<Rigidbody2D>().get();
		collider = GetComponent<Collider2D>().get();

		if (!sprite) {
			THROW_ENGINE_EXCEPTION("Projectile " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ") is missing SpriteRenderer");
		}
		if (!rigidbody) {
			THROW_ENGINE_EXCEPTION("Projectile " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ") is missing Rigidbody2D");
		}
		if (!collider) {
			THROW_ENGINE_EXCEPTION("Projectile " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ") is missing Collider2D");
		}

		ConfigureSprite();

		// Projectiles are kinematic and move via velocity.
		rigidbody->SetBodyType(Rigidbody2D::BodyType::Kinematic);
		rigidbody->SetFixedRotation(true);
		rigidbody->SetIsBullet(true);

		collider->SetTrigger(true);
		collider->SetShouldSensorEvent(true);

		if (auto* box = dynamic_cast<BoxCollider2D*>(collider)) {
			Vector2i size = sprite ? sprite->GetFrameSize() : Vector2i::Zero();
			if ((size.x <= 0 || size.y <= 0) && sprite && sprite->GetTexture()) {
				size = sprite->GetTexture()->GetSize();
			}
			if (size.x <= 0 || size.y <= 0) size = Vector2i(16, 16);
			box->SetSize(size);
		}
	}

	void Start() override {
		ApplyVelocity();

		if (m_lifetimeSeconds > 0.0f) {
			Invoke([this]() {
				Object::Destroy(GetGameObject());
			}, m_lifetimeSeconds);
		}
	}

	void OnTriggerEnter(Collider2D* other) override { HandleHit(other); }
	void OnCollisionEnter(Collider2D* other) override { HandleHit(other); }

	virtual void SpawnExplosion() {
		Scene* scene = GetGameObject()->GetScene();
		if (!scene) return;
		auto vfx = scene->CreateGameObject<ExplosionVFX>("ExplosionVFX");
		vfx->GetTransform()->SetPosition(GetTransform()->GetPosition());
	}

	void ApplyVelocity() {
		if (!rigidbody) return;
		Vector2f dir = m_direction;
		if (dir.LengthSquared() <= 0.0001f) dir = Vector2f(1.0f, 0.0f);
		dir = dir.Normalized();
		rigidbody->SetLinearVelocity(dir * m_speed);
	}

	void HandleHit(Collider2D* other) {
		if (!other) return;
		GameObject* otherGO = other->GetGameObject();
		if (!otherGO) return;
		if (otherGO == GetGameObject()) return;

		IDamageable* damageable = FindDamageable(otherGO);
		if (!damageable) return;
		if (damageable->GetFaction() == m_faction) return; // no friendly fire

		if (m_damage > 0) {
			damageable->ApplyDamage(m_damage, GetGameObject());
		}

		SpawnExplosion();
		Object::Destroy(GetGameObject());
	}

	SpriteRenderer* sprite = nullptr;
	Rigidbody2D* rigidbody = nullptr;
	Collider2D* collider = nullptr;

	Faction m_faction = Faction::Neutral;
	int m_damage = 1;
	float m_speed = 650.0f;
	float m_lifetimeSeconds = 3.0f;
	Vector2f m_direction = Vector2f(1.0f, 0.0f);
};

class PlayerProjectileBehaviour : public ProjectileBehaviour {
public:
	PlayerProjectileBehaviour()
		: ProjectileBehaviour("PlayerProjectileBehaviour") {
		SetFaction(Faction::Player);
	}
};

class EnemyProjectileBehaviour : public ProjectileBehaviour {
public:
	EnemyProjectileBehaviour()
		: ProjectileBehaviour("EnemyProjectileBehaviour") {
		SetFaction(Faction::Enemy);
	}
};

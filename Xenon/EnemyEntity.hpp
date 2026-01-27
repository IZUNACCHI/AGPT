#pragma once

#include "AllyEntity.hpp"

class EnemyEntity : public Entity {
public:
	explicit EnemyEntity()
		: Entity() {
		SetComponentName("EnemyEntity");
	}


protected:

	int m_points = 1000;
	int m_damageOnContact = 1;
	void Awake() override {
		// call base Awake
		Entity::Awake();
		// set faction to Enemy
		m_faction = Faction::Enemy;

	}

	void OnCollisionEnter(Collider2D* other) override
	{
		HandleOverlap(other);
	}


	void OnTriggerEnter(Collider2D* other) override
	{
		HandleOverlap(other);
	}

private:
	void HandleOverlap(Collider2D* other) {
		// If the other collider has a component that implements IDamageable
		// and that damageable belongs to the Player faction, apply contact damage.
		if (!m_alive) {
			return;
		}

		if (!other) {
			return;
		}

		GameObject* otherGO = other->GetGameObject();
		if (!otherGO) {
			return;
		}

		// Ignore self-collisions.
		if (otherGO == GetGameObject()) {
			return;
		}

		// Look for ANY MonoBehaviour on the other GameObject that implements IDamageable.
		IDamageable* damageable = nullptr;
		auto behaviours = otherGO->GetComponents<MonoBehaviour>();
		for (const auto& behaviour : behaviours) {
			if (!behaviour) {
				continue;
			}
			if (auto* asDamageable = dynamic_cast<IDamageable*>(behaviour.get())) {
				damageable = asDamageable;
				break;
			}
		}

		if (!damageable) {
			return;
		}

		// Only damage the player faction 
		if (damageable->GetFaction() != Faction::Player) {
			return;
		}

		if (m_damageOnContact > 0) {
			damageable->ApplyDamage(m_damageOnContact, GetGameObject());
		}
	}
};

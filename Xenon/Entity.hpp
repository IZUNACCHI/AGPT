#pragma once

#include "IDamageable.hpp"
#include "Faction.hpp"
#include "../GameEngine/GameEngine.h"
#include <string>

class Entity : public MonoBehaviour, public IDamageable {
public:
	explicit Entity(const std::string& componentName = "Entity",
		int maxHealth = 5,
		Faction faction = Faction::Enemy)
		: m_faction(faction), m_maxHealth(maxHealth), m_health(maxHealth) {
		SetComponentName(componentName);
	}
	
	// IDamageable implementation
	// Get the faction of the entity
	Faction GetFaction() const override { return m_faction; }
	// Check if the entity is alive
	bool IsAlive() const override { return m_alive; }
	// Get the current health of the entity
	int GetHealth() const override { return m_health; }
	// Set the current health of the entity
	void SetHealth(int health) {
		m_health = std::clamp(health, 0, m_maxHealth);
		if (m_health <= 0) {
			m_alive = false;
			OnDeath(nullptr);
		}
	}
	// Get the maximum health of the entity
	int GetMaxHealth() const override { return m_maxHealth; }
	// Set the maximum health of the entity
	void SetMaxHealth(int maxHealth) {
		m_maxHealth = maxHealth;
		if (m_health > m_maxHealth) {
			m_health = m_maxHealth;
		}
	}

	// Apply damage to the entity
	void ApplyDamage(int amount, GameObject* instigator = nullptr) override {
		if (!m_alive || amount <= 0) return;
		m_health = std::max(0, m_health - amount);
		if (m_health <= 0) { m_alive = false; OnDeath(instigator); }
		OnDamageTaken(amount, instigator);
		LOG_INFO("Entity " + GetGameObject()->GetName() + " took " + std::to_string(amount) + " damage from " +
			(instigator ? instigator->GetName() : "unknown") + ". Current health: " + std::to_string(m_health));
	}

	// Heal the entity
	void Heal(int amount, GameObject* instigator = nullptr) override {
		if (!m_alive || amount <= 0) return;
		m_health = std::min(m_maxHealth, m_health + amount);
		OnHeal(amount, instigator);
	}

protected:
	void Awake() override
	{
		rigidbody = GetComponent<Rigidbody2D>().get();
		if (rigidbody) {
			rigidbody->SetBodyType(Rigidbody2D::BodyType::Dynamic);
			rigidbody->SetFixedRotation(true);
		}
		else {
			THROW_ENGINE_EXCEPTION("Entity " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ")" + " is missing Rigidbody2D component");
		}
		if(collider = GetComponent<Collider2D>().get()) {
			collider->SetTrigger(true);
			collider->SetShouldSensorEvent(true);
		} else {
			THROW_ENGINE_EXCEPTION("Entity " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ")" + " is missing Collider2D component");
		}
		if(sprite = GetComponent<SpriteRenderer>().get()) {
			// SpriteRenderer found
		} else {
			THROW_ENGINE_EXCEPTION("Entity " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ")" + " is missing SpriteRenderer component");
		}
		transform = GetTransform();
	}

	void OnDamageTaken(int amount, GameObject* instigator) {
		// Can be overridden in derived classes for custom behavior
	}

	void OnHeal(int amount, GameObject* instigator) {
		// Can be overridden in derived classes for custom behavior
	}

	
	// Called when the entity dies
	virtual void OnDeath(GameObject* instigator) {
		Object::Destroy(GetGameObject()); // destroy owner object
	}

	Rigidbody2D* rigidbody = nullptr;
	SpriteRenderer* sprite = nullptr;
	Collider2D* collider = nullptr;

	Transform* transform = nullptr;

	Faction m_faction;
	int m_maxHealth;
	int m_health;
	bool m_alive = true;

	void Reset() override
	{
		m_health = m_maxHealth;
		m_alive = true;
	}

};
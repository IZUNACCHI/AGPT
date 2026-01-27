#pragma once

#include "../GameEngine/GameEngine.h"

class Projectile : public GameObject {
	public:
	explicit Projectile(const std::string& name = "Projectile")
		: GameObject(name) {
		AddComponent<Rigidbody2D>();
		AddComponent<CircleCollider2D>();
		AddComponent<ProjectileBehaviour>();
	}
	int GetDamage() const { return m_damage; }
	void SetDamage(int damage) { m_damage = damage; }
	GameObject* GetOwner() const { return m_owner; }
	void SetOwner(GameObject* owner) { m_owner = owner; }
};

class ProjectileBehaviour : public MonoBehaviour {
	SpriteRenderer* sprite = nullptr;
	Rigidbody2D* rigidbody = nullptr;
	CircleCollider2D* circleCollider = nullptr;
	Texture* projectileTexture = nullptr;
}
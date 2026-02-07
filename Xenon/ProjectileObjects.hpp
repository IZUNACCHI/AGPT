#pragma once

#include "../GameEngine/GameEngine.h"
#include "../GameEngine/Animator.h"
#include "Missiles.hpp"
#include "EnemyProjectiles.hpp"

// -----------------------------------------------------------------------------
// Projectile GameObject prefabs
// -----------------------------------------------------------------------------

class PlayerProjectile : public GameObject {
public:
	explicit PlayerProjectile(const std::string& name = "PlayerProjectile")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<Rigidbody2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<MissileBehaviour>();
	}
};

class EnemyProjectile : public GameObject {
public:
	explicit EnemyProjectile(const std::string& name = "EnemyProjectile")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<Rigidbody2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<Animator>();
		AddComponent<EnemyProjectileSheetBehaviour>();
	}
};

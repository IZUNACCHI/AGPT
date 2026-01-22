#pragma once
#include "../GameEngine/GameEngine.h"
#include "SpaceShipBehaviour.hpp"

class SpaceShip : public GameObject
{
	SpaceShip() : GameObject("SpaceShip") {
		AddComponent<SpriteRenderer>();
		AddComponent<Rigidbody2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<SpaceShipBehaviour>();
	}
};
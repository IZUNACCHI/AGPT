#pragma once
#include "../GameEngine/GameEngine.h"
#include "AllyEntity.hpp"



class SpaceShipBehaviour : public AllyEntity {
	
protected:
	Texture* shipTexture = nullptr;
	BoxCollider2D* boxCol = nullptr;

	const Vector2f moveSpeed = Vector2f(300, 280);

	void Awake() override {
		AllyEntity::Awake();

		sprite->SetTexture(LoadTexture("Ship2.bmp", Vector3i(255, 0, 255)));
		sprite->SetFrameSize(Vector2i(64, 64));
		sprite->SetFrameIndex(3);


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
	};

	void Update() override {
		// Move the spaceship based on user input
		Vector2f velocity = Vector2f::Zero();
		if (IsKeyDown(Key::W)) {
			velocity.y += 1.0f;
		}
		if (IsKeyDown(Key::S)) {
			velocity.y -= 1.0f;
		}
		if (IsKeyDown(Key::A)) {
			velocity.x -= 1.0f;
		}
		if (IsKeyDown(Key::D)) {
			velocity.x += 1.0f;
		}
		if (velocity.LengthSquared() > 0.0f) {
			velocity = velocity.Normalized() * moveSpeed;
		}

		rigidbody->SetLinearVelocity(velocity);
	}

};

class SpaceShip : public GameObject {
public:
	explicit SpaceShip(const std::string& name = "SpaceShip")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<Rigidbody2D>();
		AddComponent<BoxCollider2D>();
		AddComponent<SpaceShipBehaviour>();
	}
};

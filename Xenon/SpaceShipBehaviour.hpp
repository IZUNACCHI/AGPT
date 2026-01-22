#pragma once
#include "../GameEngine/GameEngine.h"


class SpaceShipBehaviour : public MonoBehaviour {

	SpriteRenderer* sprite = nullptr;
	Rigidbody2D* rigidbody = nullptr;
	Transform* transform = nullptr;
	BoxCollider2D* boxCollider = nullptr;
	
	const Vector2f moveSpeed = Vector2f(30000, 28000);
	
protected:
	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		rigidbody = GetComponent<Rigidbody2D>().get();
		rigidbody->SetBodyType(Rigidbody2D::BodyType::Kinematic);
		transform = GetTransform();
		boxCollider = GetComponent<BoxCollider2D>().get();

		if(sprite) {
			sprite->SetTexture(LoadTexture("Ship2.bmp", Vector3i(255, 0, 255)));
			sprite->SetFrameSize(Vector2i(64, 64));
			sprite->SetFrameIndex(3);
		}
	}

	void Update() override{
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
			velocity = velocity.Normalized() * moveSpeed * Time::DeltaTime();
		}
		if (IsKeyPressed(Key::Tab)) {
			moveSpeed += Vector2i(10, 10);
			LOG_INFO("Move Speed = " + std::to_string(moveSpeed.x) + ", " + std::to_string(moveSpeed.y));
		}
		else if (IsKeyPressed(Key::LeftShift))
		{
			moveSpeed -= Vector2i(10, 10);
		}
		rigidbody->SetLinearVelocity(velocity);
	}

	void OnCollisionEnter(Collider2D* other) override {
		if (other && other->GetGameObject()) {
			LOG_INFO("SpaceShip collision enter with " + other->GetGameObject()->GetName());
			return;
		}
		LOG_INFO("SpaceShip collision enter");
	}

	void OnTriggerEnter(Collider2D* other) override {
		if (other && other->GetGameObject()) {
			LOG_INFO("SpaceShip trigger enter with " + other->GetGameObject()->GetName());
			return;
		}
		LOG_INFO("SpaceShip trigger enter");
	}

};
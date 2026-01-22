#pragma once
#include "../GameEngine/GameEngine.h"


class SpaceShipBehaviour : public MonoBehaviour {

	SpriteRenderer* sprite = nullptr;
	Rigidbody2D* rigidbody = nullptr;
	Transform* transform = nullptr;
	BoxCollider2D* boxCollider = nullptr;
	Vector2f velocity = Vector2f::Zero();
	
protected:
	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		rigidbody = GetComponent<Rigidbody2D>().get();
		transform = GetTransform();
		boxCollider = GetComponent<BoxCollider2D>().get();
	}

	void Update(){
		// Move the spaceship based on user input
		const float moveSpeed = 200.0f;
		if (Input::IsKeyPressed(Key::W)) {
			velocity.y += 1.0f;
		}
		if (Input::IsKeyPressed(Key::S)) {
			velocity.y -= 1.0f;
		}
		if (Input::IsKeyPressed(Key::A)) {
			velocity.x -= 1.0f;
		}
		if (Input::IsKeyPressed(Key::D)) {
			velocity.x += 1.0f;
		}
		if (velocity.LengthSquared() > 0.0f) {
			velocity = velocity.Normalized() * moveSpeed;
		}
		rigidbody->SetLinearVelocity(velocity);
	}
};
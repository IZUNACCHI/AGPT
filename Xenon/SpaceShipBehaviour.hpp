#pragma once
#include "../GameEngine/GameEngine.h"


class SpaceShipBehaviour : public MonoBehaviour {

	SpriteRenderer* sprite = nullptr;
	Texture* shipTexture = nullptr;
	Rigidbody2D* rigidbody = nullptr;
	Transform* transform = nullptr;
	BoxCollider2D* boxCollider = nullptr;
	
	const Vector2f moveSpeed = Vector2f(300, 280);
	
protected:
	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		rigidbody = GetComponent<Rigidbody2D>().get();
		transform = GetTransform();
		boxCollider = GetComponent<BoxCollider2D>().get();

		if(sprite) {
			shipTexture = LoadTexture("Ship2.bmp", Vector3i(255, 0, 255));
			sprite->SetTexture(shipTexture);
			sprite->SetFrameSize(Vector2i(64, 64));
			sprite->SetFrameIndex(3);
		}
	}

	void Update() override{
		// Move the spaceship based on user input
		Vector2f velocity = Vector2f::Zero();
		if (IsKeyDown(Key::W)) {
			velocity.y -= 1.0f;
		}
		if (IsKeyDown(Key::S)) {
			velocity.y += 1.0f;
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
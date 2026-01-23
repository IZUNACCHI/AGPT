#pragma once
#include "../GameEngine/GameEngine.h"

class BumperBehaviour : public MonoBehaviour {
	SpriteRenderer* sprite = nullptr;
	BoxCollider2D* boxCollider = nullptr;
	Texture* bumperTexture = nullptr;
	Rigidbody2D* rigidbody = nullptr;

protected:
	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		rigidbody = GetComponent<Rigidbody2D>().get();
		boxCollider = GetComponent<BoxCollider2D>().get();

		if(rigidbody) {
			rigidbody->SetBodyType(Rigidbody2D::BodyType::Dynamic);
		}

		if (sprite) {
			bumperTexture = LoadTexture("Burner1.bmp", Vector3i(255, 0, 255));
			sprite->SetTexture(bumperTexture);
			sprite->SetLayerOrder(-2);
		}

		if (boxCollider) {
			boxCollider->SetSize(Vector2(32, 32));
			boxCollider->SetRestitution(0.85f);
			boxCollider->SetFriction(0.1f);
		}
	}
};

class Bumper : public GameObject {
public:
	explicit Bumper(const std::string& name = "Bumper")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<BoxCollider2D>();
		AddComponent<BumperBehaviour>();
	}
};

class OverlapZoneBehaviour : public MonoBehaviour {
	SpriteRenderer* sprite = nullptr;
	BoxCollider2D* boxCollider = nullptr;
	Texture* overlapTexture = nullptr;

protected:
	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		boxCollider = GetComponent<BoxCollider2D>().get();

		if (sprite) {
			overlapTexture = LoadTexture("destroyable.bmp", Vector3i(255, 0, 255));
			sprite->SetTexture(overlapTexture);
			sprite->SetLayerOrder(-1);
		}

		if (boxCollider) {
			boxCollider->SetSize(Vector2(32, 32));
			boxCollider->SetTrigger(true);
		}
	}
};

class OverlapZone : public GameObject {
public:
	explicit OverlapZone(const std::string& name = "OverlapZone")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<BoxCollider2D>();
		AddComponent<OverlapZoneBehaviour>();
	}
};

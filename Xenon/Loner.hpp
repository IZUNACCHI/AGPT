#pragma once
#include "../GameEngine/GameEngine.h"
#include "EnemyEntity.hpp"

class LonerBehaviour : public EnemyEntity {
	Texture* lonerTexture = nullptr;
	BoxCollider2D* boxCol = nullptr;


protected:
	void Awake() override {
		EnemyEntity::Awake();

		if (rigidbody) {
			rigidbody->SetBodyType(Rigidbody2D::BodyType::Dynamic);
		}

		lonerTexture = LoadTexture("LonerA.bmp", Vector3i(255, 0, 255));
		sprite->SetTexture(lonerTexture);
		sprite->SetFrameSize(Vector2i(64, 64));
		sprite->SetFrameIndex(0);
		sprite->SetLayerOrder(-2);

		boxCol = dynamic_cast<BoxCollider2D*>(collider);
		if (boxCol) {
			boxCol->SetSize(sprite->GetFrameSize());
			boxCol->SetTrigger(true);
			boxCol->SetShouldSensorEvent(true);
		}
		else {
			THROW_ENGINE_EXCEPTION("Entity " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ")" + " is missing BoxCollider2D component");
		}
	}
};

class Loner : public GameObject {
public:
	explicit Loner(const std::string& name = "Loner")
		: GameObject(name) {
		AddComponent<Rigidbody2D>();
		AddComponent<SpriteRenderer>();
		AddComponent<BoxCollider2D>();
		AddComponent<LonerBehaviour>();
	}
};
#pragma once

#include <GameEngine/GameEngine.h>
#include "XenonViewportComponents.hpp"
#include "Entity.hpp"

class PickupBehaviour : public MonoBehaviour {
public:
	PickupBehaviour()
		: MonoBehaviour() {
		SetComponentName("PickupBehaviour");
	}

protected:
	virtual bool ApplyTo(Entity* ally) { (void)ally; return true; }

	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		rigidbody = GetComponent<Rigidbody2D>().get();
		collider = GetComponent<Collider2D>().get();

		if (!sprite) {
			THROW_ENGINE_EXCEPTION("Pickup is missing SpriteRenderer component");
		}
		if (!rigidbody) {
			THROW_ENGINE_EXCEPTION("Pickup is missing Rigidbody2D component");
		}
		if (!collider) {
			THROW_ENGINE_EXCEPTION("Pickup is missing Collider2D component");
		}

		rigidbody->SetBodyType(Rigidbody2D::BodyType::Kinematic);
		rigidbody->SetFixedRotation(true);

		collider->SetTrigger(true);
		collider->SetShouldSensorEvent(true);

		if (GetTransform()) {
			GetTransform()->SetRotation(-90.0f);
		}

		if (auto* box = dynamic_cast<BoxCollider2D*>(collider)) {
			Vector2f size(32.0f, 32.0f);
			if (sprite->GetFrameSize().x > 0 && sprite->GetFrameSize().y > 0) {
				size = Vector2f((float)sprite->GetFrameSize().x, (float)sprite->GetFrameSize().y);
			}
			box->SetSize(size);
		}
	}

	void Update() override {
		if (rigidbody && GetTransform()) {
			const Vector2f dir = -GetTransform()->GetUp();
			rigidbody->SetLinearVelocity(dir * m_speed);
		}

		// Offscreen despawn is handled by DespawnOffscreen2D.
	}

	void OnTriggerEnter(Collider2D* other) override {
		if (m_consumed) return;
		if (!other) return;

		GameObject* otherGO = other->GetGameObject();
		if (!otherGO) return;

		Entity* ally = nullptr;
		if (auto ent = otherGO->GetComponent<Entity>()) {
			ally = ent.get();
		}
		if (!ally) return;
		if (ally->GetFaction() != Faction::Player) return;

		if (!ApplyTo(ally)) {
			return;
		}

		m_consumed = true;
		Object::Destroy(GetGameObject());
	}

public:
	void SetSpeed(float s) { m_speed = s; }

protected:
	SpriteRenderer* sprite = nullptr;
	Rigidbody2D* rigidbody = nullptr;
	Collider2D* collider = nullptr;

private:
	bool m_consumed = false;
	float m_speed = 120.0f;
};

class Pickup : public GameObject {
public:
	explicit Pickup(const std::string& name = "Pickup")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<Rigidbody2D>();
		AddComponent<DespawnOffscreen2D>();
		AddComponent<BoxCollider2D>();
	}
};

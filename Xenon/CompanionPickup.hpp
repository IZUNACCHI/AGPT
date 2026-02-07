#pragma once

#include "Pickup.hpp"
#include "SpaceShip.hpp"

// -----------------------------------------------------------------------------
// Companion pickup
// - Only the Spaceship can pick this up.
// - Companions must NOT pick this up.
// - Consumed only if it successfully adds a companion (max 2).
// -----------------------------------------------------------------------------

class CompanionPickupBehaviour : public PickupBehaviour {
public:
	CompanionPickupBehaviour()
		: PickupBehaviour() {
		SetComponentName("CompanionPickupBehaviour");
	}

protected:
	void Awake() override {
		// Simple visual: use the first frame of clone.bmp as the pickup icon.
		auto spr = GetComponent<SpriteRenderer>().get();
		if (spr) {
			spr->SetTexture(LoadTexture("clone.bmp", Vector3i(255, 0, 255)));
			spr->SetFrameSize(Vector2i(32, 32));
			spr->SetFrameIndex(0);
			spr->SetLayerOrder(2);
		}

		PickupBehaviour::Awake();
	}

	bool ApplyTo(Entity* ally) override {
		if (!ally || !ally->IsAlive()) return false;
		GameObject* allyGO = ally->GetGameObject();
		if (!allyGO) return false;

		// Only the SpaceShip can pick it up.
		auto shipBehaviour = allyGO->GetComponent<SpaceShipBehaviour>();
		if (!shipBehaviour) {
			return false; // don't consume
		}

		// Only consume if we actually add a companion.
		return shipBehaviour->TryAddCompanion();
	}
};

class CompanionPickup : public Pickup {
public:
	explicit CompanionPickup(const std::string& name = "CompanionPickup")
		: Pickup(name) {
		AddComponent<CompanionPickupBehaviour>();
	}
};

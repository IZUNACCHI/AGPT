#pragma once

#include "AnimatedPickup.hpp"

#include <algorithm>

// -----------------------------------------------------------------------------
// Shield / Heal pickup
// - Restores health of the ally that touches it.
// - Uses Xenon asset: PUShield.bmp
// -----------------------------------------------------------------------------

class HealPickupBehaviour : public AnimatedPickupBehaviour {
public:
	HealPickupBehaviour()
		: AnimatedPickupBehaviour() {
		SetComponentName("HealPickupBehaviour");
		// Visual: loop the entire PUShield spritesheet (if it's only 1 frame,
		// this will just display it statically).
		SetSpriteSheetPath("PUShield.bmp");
		SetFrameSize(Vector2i(32, 32));
		SetFPS(10.0f);
		SetLayerOrder(2);
		SetColorKey(Vector3i(255, 0, 255));
	}

	void SetHealAmount(int amount) { m_healAmount = std::max(0, amount); }
	int GetHealAmount() const { return m_healAmount; }

protected:
	bool ApplyTo(Entity* ally) override {
		if (!ally) return false;
		if (!ally->IsAlive()) return false;
		ally->Heal(m_healAmount, GetGameObject());
		return true;
	}

private:
	int m_healAmount = 25;
};

class HealPickup : public AnimatedPickup {
public:
	explicit HealPickup(const std::string& name = "HealPickup")
		: AnimatedPickup(name) {
		AddComponent<HealPickupBehaviour>();
	}
};

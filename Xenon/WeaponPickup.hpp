#pragma once

#include "AnimatedPickup.hpp"
#include "ProjectileLaunchers.hpp" // PlayerProjectileLauncher + MissileType

// -----------------------------------------------------------------------------
// Weapon pickup
// - Upgrades the missile tier of the ally that touches it.
// - Uses Xenon asset: PUWeapon.bmp
// - Light -> Medium -> Heavy (stays at Heavy)
// -----------------------------------------------------------------------------

inline MissileType NextMissileType(MissileType current) {
	switch (current) {
	case MissileType::Light:  return MissileType::Medium;
	case MissileType::Medium: return MissileType::Heavy;
	case MissileType::Heavy:  return MissileType::Heavy;
	default:                 return MissileType::Light;
	}
}

class WeaponPickupBehaviour : public AnimatedPickupBehaviour {
public:
	WeaponPickupBehaviour()
		: AnimatedPickupBehaviour() {
		SetComponentName("WeaponPickupBehaviour");
		// Visual: loop the entire PUWeapon spritesheet (if it's only 1 frame,
		// this will just display it statically).
		SetSpriteSheetPath("PUWeapon.bmp");
		SetFrameSize(Vector2i(32, 32));
		SetFPS(10.0f);
		SetLayerOrder(2);
		SetColorKey(Vector3i(255, 0, 255));
	}

protected:
	bool ApplyTo(Entity* ally) override {
		if (!ally) return false;
		if (!ally->IsAlive()) return false;

		GameObject* allyGO = ally->GetGameObject();
		if (!allyGO) return false;

		auto launcher = allyGO->GetComponent<PlayerProjectileLauncher>();
		if (!launcher) {
			// Ally can't shoot (or doesn't use missiles)  don't consume.
			return false;
		}

		MissileType current = launcher->GetMissileType();
		launcher->SetMissileType(NextMissileType(current));
		return true;
	}
};

class WeaponPickup : public AnimatedPickup {
public:
	explicit WeaponPickup(const std::string& name = "WeaponPickup")
		: AnimatedPickup(name) {
		AddComponent<WeaponPickupBehaviour>();
	}
};

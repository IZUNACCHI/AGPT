#pragma once
#include "Faction.hpp"

#include "../GameEngine/GameEngine.h"

class IDamageable {
public:
	virtual ~IDamageable() = default;
	virtual Faction GetFaction() const = 0;
	virtual bool IsAlive() const = 0;
	virtual int GetHealth() const = 0;
	virtual int GetMaxHealth() const = 0;
	virtual void ApplyDamage(int amount, GameObject* instigator = nullptr) = 0;
	virtual void Heal(int amount, GameObject* instigator = nullptr) = 0;
};
#pragma once

#include "Entity.hpp"

class AllyEntity : public Entity {
public:
	explicit AllyEntity()
		: Entity() {
		SetComponentName("AllyEntity");
	}


protected:
	void Awake() override {
		// call base Awake
		Entity::Awake();
		// set faction to Enemy
		m_faction = Faction::Player;

	}

};

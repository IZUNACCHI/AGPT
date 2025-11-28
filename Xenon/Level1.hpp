#pragma once
#include "Scene.h"
#include "GameObjects.hpp"
#include <memory>

class SpaceShip;

class Level1 : public Scene {
public:
	void OnLoad() override;
private:


};

void Level1::OnLoad()
{
	auto ship = std::make_unique<SpaceShip>();
	AddGameObject(std::move(ship));
}



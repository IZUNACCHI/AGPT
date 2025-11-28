#pragma once
#include "Scene.h"
#include "GameObjects.hpp"

class Level1 : public Scene {
public:
	void OnLoad() override;
private:


};

void Level1::OnLoad()
{
	CreateGameObject<SpaceShip>("Player");


}



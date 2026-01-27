#include <memory>
#include "SpaceShip.hpp"
#include "Loner.hpp"

class Level1 : public Scene {
public:
	Level1()
		: Scene("Level1") {
	}

	void OnStart() override {

		auto spaceShip = CreateGameObject<SpaceShip>("SpaceShip");
		spaceShip->GetTransform()->SetPosition(Vector2f::Zero());

		auto loner1 = CreateGameObject<Loner>("Loner1");
		loner1->GetTransform()->SetPosition(Vector2f(-200.0f, 0.0f));
	}

};
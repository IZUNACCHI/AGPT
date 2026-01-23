#include <memory>
#include "SpaceShip.hpp"
#include "Bumper.hpp"

class Level1 : public Scene {
public:
	Level1()
		: Scene("Level1") {
	}

	void OnStart() override {

		auto spaceShip = CreateGameObject<SpaceShip>("SpaceShip");
		spaceShip->GetTransform()->SetPosition(Vector2f::Zero());

		auto bumper = CreateGameObject<Bumper>("Bumper");
		bumper->GetTransform()->SetPosition(Vector2(100, 100));
		

		auto overlapZone = CreateGameObject<OverlapZone>("OverlapZone");
		overlapZone->GetTransform()->SetPosition(Vector2(-100, -100));
	}

};
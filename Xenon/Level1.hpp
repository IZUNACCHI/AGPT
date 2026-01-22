#include <memory>
#include "SpaceShip.hpp"
#include "Bumper.hpp"

class Level1 : public Scene {
public:
	Level1()
		: Scene("Level1") {
	}

	void OnStart() override {
		const Vector2f windowCenter = GetWindow()->GetSize() / 2;

		auto spaceShip = CreateGameObject<SpaceShip>("SpaceShip");
		spaceShip->GetTransform()->SetPosition(windowCenter);

		auto bumper = CreateGameObject<Bumper>("Bumper");
		bumper->GetTransform()->SetPosition(windowCenter + Vector2f(0.9f, 0.0f));

		auto overlapZone = CreateGameObject<OverlapZone>("OverlapZone");
		overlapZone->GetTransform()->SetPosition(windowCenter + Vector2f(-0.1f, 0.0f));
	}

};
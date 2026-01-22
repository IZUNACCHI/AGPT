#include <memory>
#include "SpaceShip.hpp"

class Level1 : public Scene {
public:
	Level1()
		: Scene("Level1") {
	}

	void OnStart() override {
		auto spaceShip = CreateGameObject<SpaceShip>("SpaceShip");
		spaceShip->GetTransform()->SetPosition(GetWindow()->GetSize() / 2);
	}

private:
	std::unique_ptr<Texture> m_texture;
};
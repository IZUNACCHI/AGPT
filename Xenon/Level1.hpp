// Xenon/Level1.hpp
#include <memory>

#include "SpaceShip.hpp"

#include "EnemySpawners.hpp"
#include "Pickups.hpp"

#include "XenonGameMode.hpp"
#include "XenonHUDController.hpp"

class Level1 : public Scene {
public:
	Level1()
		: Scene("Level1") {
		SetGameMode<XenonGameMode>();
	}

	void OnStart() override {
		{
			auto hud = CreateGameObject<GameObject>("HUDController");
			hud->AddComponent<XenonHUDController>();
		}

		{
			auto galaxy = CreateGameObject<GameObject>("GalaxyBackdrop");
			auto galaxySprite = galaxy->AddComponent<SpriteRenderer>();
			galaxySprite->SetTexture(LoadTexture("galaxy2.bmp"));
			galaxySprite->SetLayerOrder(-10);
			galaxy->GetTransform()->SetPosition(Vector2f(0.0f, 0.0f));
		}

		auto spaceShip = CreateGameObject<SpaceShip>("SpaceShip");
		spaceShip->GetTransform()->SetPosition(Vector2f(0.0f, -160.0f));
		spaceShip->GetTransform()->SetRotation(-90.0f);

		{
			auto go = CreateGameObject<GameObject>("Spawner_LonerWave");
			auto sp = go->AddComponent<LonerWaveSpawnerBehaviour>();
			sp->SetKey(Key::Num1);
			sp->SetCooldown(0.15f);
			sp->SetCount(2);
			sp->SetSpacing(100.0f);
			sp->SetForwardDistance(200.0f);
		}

		{
			auto go = CreateGameObject<GameObject>("Spawner_RusherWave");
			auto sp = go->AddComponent<RusherWaveSpawnerBehaviour>();
			sp->SetKey(Key::Num2);
			sp->SetCooldown(0.15f);
			sp->SetCount(6);
			sp->SetSpacing(42.0f);
			sp->SetForwardDistance(520.0f);
		}

		{
			auto go = CreateGameObject<GameObject>("Spawner_DronePack");
			auto sp = go->AddComponent<DronePackSpawnerBehaviour>();
			sp->SetKey(Key::Num3);
			sp->SetCooldown(0.15f);
			sp->SetPackSize(6);
			sp->SetSpacing(44.0f);
			sp->SetForwardDistance(520.0f);
			sp->SetForwardSpeed(140.0f);
			sp->SetSin(55.0f, 0.75f);
		}

		{
			auto go = CreateGameObject<GameObject>("Spawner_StoneAsteroid");
			auto sp = go->AddComponent<StoneAsteroidSpawnerBehaviour>();
			sp->SetKey(Key::Num4);
			sp->SetCooldown(0.15f);
			sp->SetSize(StoneAsteroidSize::Large96);
			sp->SetCount(1);
			sp->SetForwardDistance(520.0f);
		}

		{
			auto go = CreateGameObject<GameObject>("Spawner_MetalAsteroid");
			auto sp = go->AddComponent<MetalAsteroidSpawnerBehaviour>();
			sp->SetKey(Key::Num5);
			sp->SetCooldown(0.15f);
			sp->SetSize(MetalAsteroidSize::Large96);
			sp->SetCount(1);
			sp->SetForwardDistance(520.0f);
		}

		{
			auto go = CreateGameObject<GameObject>("Spawner_Pickups");
			auto sp = go->AddComponent<PickupKeySpawnerBehaviour>();
			sp->SetHealKey(Key::Num6);
			sp->SetWeaponKey(Key::Num7);
			sp->SetCompanionKey(Key::Num8);
			sp->SetForwardDistance(520.0f);
		}
	}
};

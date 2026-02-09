// Xenon/Level1.hpp
#pragma once

#include <memory>
#include <vector>
#include <cstdlib>

#include <GameEngine/GameEngine.h>

#include "SpaceShip.hpp"
#include "Loner.hpp"
#include "Rusher.hpp"

#include "XenonGameMode.hpp"
#include "XenonHUDController.hpp"
#include "ParallaxBackground.hpp"
#include "XenonViewportComponents.hpp"
#include "XenonAssetKeys.h"

// -----------------------------------------------------------------------------
// Project 1: simple director that spawns the required enemy types automatically.
// - Loner: moves horizontally and bounces on left/right edges; fires toward ship.
// - Rusher: moves vertically down the screen.
// -----------------------------------------------------------------------------
class Project1EnemyDirector : public MonoBehaviour {
public:
	Project1EnemyDirector() { SetComponentName("Project1EnemyDirector"); }

	void Start() override {
		// Seed once per run (ok for a simple student project).
		static bool seeded = false;
		if (!seeded) {
			seeded = true;
			std::srand((unsigned int)(Time::Now() * 1000.0f));
		}

		// Start after a short delay so the ship exists.
		InvokeRepeating([this]() { SpawnLoner(); }, 0.75f, 5.0f, MonoBehaviour::InvokeTickPolicy::WhileBehaviourEnabled);
		InvokeRepeating([this]() { SpawnRusherWave(); }, 1.25f, 2.0f, MonoBehaviour::InvokeTickPolicy::WhileBehaviourEnabled);
	}

private:
	static float Rand01() {
		return (float)std::rand() / (float)RAND_MAX;
	}

	static float RandRange(float a, float b) {
		return a + (b - a) * Rand01();
	}

	Viewport::Bounds GetBounds() const {
		Renderer* r = GetRenderer();
		if (!r) return Viewport::Bounds{};
		const Vector2i vr = r->GetVirtualResolution();
		return Viewport::GetWorldBounds(vr);
	}

	void SpawnLoner() {
		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;

		auto ship = Scene::FindGameObject("SpaceShip");
		if (!ship || !ship->IsActiveInHierarchy()) return;

		const Viewport::Bounds b = GetBounds();
		const float y = b.top - 120.0f; // near the top
		const float x = RandRange(b.left + 80.0f, b.right - 80.0f);

		auto e = scene->CreateGameObject<Loner>("Loner");
		e->GetTransform()->SetPosition(Vector2f(x, y));

		// Randomize starting horizontal direction.
		if (auto rb = e->GetComponent<Rigidbody2D>()) {
			const float dir = (Rand01() < 0.5f) ? -1.0f : 1.0f;
			rb->SetLinearVelocity(Vector2f(dir * 120.0f, 0.0f));
		}
	}

	void SpawnRusherWave() {
		Scene* scene = GetGameObject() ? GetGameObject()->GetScene() : nullptr;
		if (!scene) return;

		auto ship = Scene::FindGameObject("SpaceShip");
		if (!ship || !ship->IsActiveInHierarchy()) return;

		const Viewport::Bounds b = GetBounds();

		// Spawn 2..4 rushers spaced horizontally.
		const int count = 2 + (std::rand() % 3);
		const float spacing = 64.0f;
		const float baseX = RandRange(b.left + 120.0f, b.right - 120.0f);
		const float y = b.top + 80.0f; // just above the top edge

		for (int i = 0; i < count; ++i) {
			const float off = (float)i - (float)(count - 1) * 0.5f;
			auto e = scene->CreateGameObject<Rusher>("Rusher");
			e->GetTransform()->SetPosition(Vector2f(baseX + off * spacing, y));
		}
	}
};

class Level1 : public Scene {
public:
	Level1() : Scene("Level1") {
		SetGameMode<XenonGameMode>();
	}

	void OnStart() override {
		// HUD (optional for Project 1, but harmless).
		{
			auto hud = CreateGameObject<GameObject>("HUDController");
			hud->AddComponent<XenonHUDController>();
		}

		// Galaxy backdrop.
		{
			auto galaxy = CreateGameObject<GameObject>("GalaxyBackdrop");
			auto galaxySprite = galaxy->AddComponent<SpriteRenderer>();
			galaxySprite->SetTexture(LoadTexture(XenonAssetKeys::Files::GalaxyBmp));
			galaxySprite->SetLayerOrder(-20);
			galaxy->GetTransform()->SetPosition(Vector2f(0.0f, 0.0f));
		}

		// Demonstrate sprited tiled maps: a simple border built from Blocks.bmp.
		{
			Texture* blocks = LoadTexture(XenonAssetKeys::Files::BlocksBmp, Vector3i(255, 0, 255));
			if (blocks) {
				auto map = CreateGameObject<GameObject>("Tilemap_Background");
				auto tm = map->AddComponent<Tilemap>();
				tm->SetTileset(blocks, Vector2i(32, 32), 16);
				tm->SetLayerOrder(-15);
				tm->SetBuildOnStart(false);

				const int w = 20; // 640 / 32
				const int h = 15; // 480 / 32
				tm->SetMapSize(w, h);

				std::vector<int> tiles(w * h, -1);
				for (int x = 0; x < w; ++x) {
					tiles[0 * w + x] = (x % 16);               // top border (row 0)
					tiles[(h - 1) * w + x] = 16 + (x % 16);    // bottom border (row 1)
				}
				for (int y = 0; y < h; ++y) {
					tiles[y * w + 0] = 32 + (y % 16);          // left border (row 2)
					tiles[y * w + (w - 1)] = 48 + (y % 16);    // right border (row 3)
				}

				// Some decorative tiles in the middle.
				for (int y = 3; y < h - 3; y += 3) {
					for (int x = 3; x < w - 3; x += 5) {
						tiles[y * w + x] = 64 + ((x + y) % 16);
					}
				}

				tm->SetTiles(tiles);
				tm->Rebuild();
			}
		}

		// Parallax blocks (from Blocks.bmp atlas, specified in frames)
		{
			auto blocks1 = CreateBlocksRect(this, "BlocksRect1", 20, 1, 9, 2, Vector2f(-200.0f, 180.0f), -9);
			if (blocks1) {
				auto p = blocks1->AddComponent<ParallaxMover2D>();
				p->SetDirection(Vector2f(0.0f, -1.0f));
				p->SetSpeed(18.0f);
			}
		}
		{
			auto blocks2 = CreateBlocksRect(this, "BlocksRect2", 43, 4, 9, 2, Vector2f(140.0f, 40.0f), -8);
			if (blocks2) {
				auto p = blocks2->AddComponent<ParallaxMover2D>();
				p->SetDirection(Vector2f(0.0f, -1.0f));
				p->SetSpeed(28.0f);
			}
		}

		// Player ship.
		{
			auto spaceShip = CreateGameObject<SpaceShip>("SpaceShip");
			spaceShip->GetTransform()->SetPosition(Vector2f(0.0f, -160.0f));
			spaceShip->GetTransform()->SetRotation(0.0f);
		}

		// Enemy director.
		{
			auto go = CreateGameObject<GameObject>("EnemyDirector");
			go->AddComponent<Project1EnemyDirector>();
		}
	}
};

#pragma once
#include "GameObject.h"
#include "SpriteRenderer.h"
#include "AnimationSystem.hpp"
#include "Transform.h"
#include "Input.h"
#include "AssetManager.h"

#pragma region SpaceShip

class SpaceShip : public GameObject
{
public:
	Transform* tr;
	SpriteRenderer* sprite;
	Animator* animator;
	float speedY = 100.0f;
	float speedX = 150.0f;

	SpaceShip(const std::string& name = "SpaceShip") : GameObject(name) {
		tr = this->GetComponent<Transform>();
		tr->SetWorldPosition(Vec2(50.0f, 50.0f));
		sprite = this->AddComponent<SpriteRenderer>();
		sprite->texture = AssetManager::LoadTexture("graphics/ship1.bmp");
		//animator = this->AddComponent<Animator>();
		//animator->graph = std::make_unique<AnimationGraph>();
	}

	void OnInit() override
	{
		
		
	}

	void OnUpdate(float deltaTime) override
	{
		// Update code for SpaceShip

		// read input and move the spaceship
		if (Input::IsKeyDown(Key::A) || Input::IsGamepadButtonDown(GamepadButton::West))
		{
			tr->MoveLocal(-speedX * deltaTime, 0.0f);
		}
		if (Input::IsKeyDown(Key::D) || Input::IsGamepadButtonDown(GamepadButton::East))
		{
			tr->MoveLocal(-speedX * deltaTime, 0.0f);
		}
		if (Input::IsKeyDown(Key::W) || Input::IsGamepadButtonDown(GamepadButton::North))
		{
			tr->MoveLocal(0.0f, speedX * deltaTime);
		}
		if (Input::IsKeyDown(Key::S) || Input::IsGamepadButtonDown(GamepadButton::South))
		{
			tr->MoveLocal(0.0f, -speedX * deltaTime);
		}
	}

	void OnDestroy() override
	{
		// Cleanup code for SpaceShip
	}

private:

};

#pragma endregion




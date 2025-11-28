#pragma once
#include "GameObject.h"
#include "SpriteRenderer.h"
#include "AnimationSystem.hpp"
#include "Transform.h"
#include "Input.h"
#include "AssetManager.h"

#pragma region SpaceShip
//h
class SpaceShip : public GameObject
{
public:
	Transform* transform;
	SpriteRenderer* sprite;
	Animator* animator;
	float speedY = 100.0f;
	float speedX = 150.0f;

	void OnInit() override
	{
		transform = this->GetComponent<Transform>();
		sprite = this->AddComponent<SpriteRenderer>();
		//sprite->texture = AssetManager::LoadTexture("graphics/ship1.bmp");
		//animator = this->AddComponent<Animator>();
		//animator->graph = std::make_unique<AnimationGraph>();
		
	}

	void OnUpdate(float deltaTime) override
	{
		// Update code for SpaceShip

		// read input and move the spaceship
		if (Input::IsKeyDown(Key::A) || Input::IsGamepadButtonDown(GamepadButton::West))
		{
			transform->localPositionX -= speedX * deltaTime;
		}
		if (Input::IsKeyDown(Key::D) || Input::IsGamepadButtonDown(GamepadButton::East))
		{
			transform->localPositionX += speedX * deltaTime;
		}
		if (Input::IsKeyDown(Key::W) || Input::IsGamepadButtonDown(GamepadButton::North))
		{
			transform->localPositionY += speedY * deltaTime;
		}
		if (Input::IsKeyDown(Key::S) || Input::IsGamepadButtonDown(GamepadButton::South))
		{
			transform->localPositionY -= speedY * deltaTime;
		}
	}

	void OnDestroy() override
	{
		// Cleanup code for SpaceShip
	}

private:

};

#pragma endregion




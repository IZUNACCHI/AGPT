#pragma once
#include "GameObject.h"
#include "SpriteRenderer.h"
#include "AnimationSystem.hpp"
#include "Transform.h"
#include "Input.h"

#pragma region SpaceShip
//h
class SpaceShip : public GameObject
{
public:
	Transform* transform;
	SpriteRenderer* sprite;
	Animator* animator;

	void OnInit() override
	{
		transform = this->GetComponent<Transform>();
		sprite = this->AddComponent<SpriteRenderer>();
		animator = this->AddComponent<Animator>();
	}

	void OnUpdate(float deltaTime) override
	{
		// Update code for SpaceShip

		// read input and move the spaceship
		if (Input::IsKeyDown(Key::A) || Input::IsGamepadButtonDown(GamepadButton::West))
		{
			transform->localPositionX -= 200.0f * deltaTime;
		}
	}

	void OnDestroy() override
	{
		// Cleanup code for SpaceShip
	}

private:

};

#pragma endregion




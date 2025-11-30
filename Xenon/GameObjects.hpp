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
	float speedY = 300.f;
	float speedX = 300.f;
	Vec2 movement = Vec2::Zero();
	bool movingLeft = false;
	bool movingRight = false;
	bool movingUp = false;
	bool movingDown = false;

	void OnInit() override
	{
		tr = this->GetComponent<Transform>();
		tr->SetWorldPosition(Vec2(50.0f, 50.0f));
		sprite = this->AddComponent<SpriteRenderer>();
		sprite->texture = AssetManager::LoadTexture("../Dist/graphics/Ship1.bmp");
		sprite->sortingLayer = 1;

		// Setup animator
		animator = this->AddComponent<Animator>();
		animator->graph = std::make_unique<AnimationGraph>();
		// Common settings
		Texture* tex = nullptr;  // Use sprite->texture
		static float fw = 64.f; //in this case these don't change at all.
		static float fh = 64.f;

		// Idle animation (frame 4, col 3, single frame, looping for stability)
		auto idleAnim = std::make_unique<Animation>();
		idleAnim->name = "idle";
		idleAnim->texture = tex;
		idleAnim->frameWidth = fw;
		idleAnim->frameHeight = fh;
		idleAnim->loop = true;
		idleAnim->transitionsOnlyAtEnd = false;
		idleAnim->frames.push_back({ 3, 0, 0.1f });  // Frame 4 (0-based col 3)
		animator->graph->AddState("idle", std::move(idleAnim));

		// moveLeft animation (frames 3,2,1 -> cols 2,1,0, not looping, transitions only at end)
		auto leftAnim = std::make_unique<Animation>();
		leftAnim->name = "moveLeft";
		leftAnim->texture = tex;
		leftAnim->frameWidth = fw;
		leftAnim->frameHeight = fh;
		leftAnim->loop = false;
		leftAnim->transitionsOnlyAtEnd = false;
		leftAnim->frames.push_back({ 2, 0, 0.1f });  // Frame 3 (col 2)
		leftAnim->frames.push_back({ 1, 0, 0.1f });  // Frame 2 (col 1)
		leftAnim->frames.push_back({ 0, 0, 0.1f });  // Frame 1 (col 0)
		animator->graph->AddState("moveLeft", std::move(leftAnim));

		// moveRight animation (frames 4,5,6 -> cols 3,4,5, not looping, transitions only at end)
		auto rightAnim = std::make_unique<Animation>();
		rightAnim->name = "moveRight";
		rightAnim->texture = tex;
		rightAnim->frameWidth = fw;
		rightAnim->frameHeight = fh;
		rightAnim->loop = false;
		rightAnim->transitionsOnlyAtEnd = false;
		rightAnim->frames.push_back({ 3, 0, 0.1f });  // Frame 4 (col 3)
		rightAnim->frames.push_back({ 4, 0, 0.1f });  // Frame 5 (col 4)
		rightAnim->frames.push_back({ 5, 0, 0.1f });  // Frame 6 (col 5)
		animator->graph->AddState("moveRight", std::move(rightAnim));

		// leftToIdle animation (frames 1,2,3,4 -> cols 0,1,2,3, not looping, transitions only at end)
		auto leftToIdleAnim = std::make_unique<Animation>();
		leftToIdleAnim->name = "leftToIdle";
		leftToIdleAnim->texture = tex;
		leftToIdleAnim->frameWidth = fw;
		leftToIdleAnim->frameHeight = fh;
		leftToIdleAnim->loop = false;
		leftToIdleAnim->transitionsOnlyAtEnd = false;
		leftToIdleAnim->frames.push_back({ 0, 0, 0.1f });  // Frame 1 (col 0)
		leftToIdleAnim->frames.push_back({ 1, 0, 0.1f });  // Frame 2 (col 1)
		leftToIdleAnim->frames.push_back({ 2, 0, 0.1f });  // Frame 3 (col 2)
		leftToIdleAnim->frames.push_back({ 3, 0, 0.1f });  // Frame 4 (col 3)
		animator->graph->AddState("leftToIdle", std::move(leftToIdleAnim));

		// rightToIdle animation (frames 6,5,4 -> cols 5,4,3, not looping, transitions only at end)
		auto rightToIdleAnim = std::make_unique<Animation>();
		rightToIdleAnim->name = "rightToIdle";
		rightToIdleAnim->texture = tex;
		rightToIdleAnim->frameWidth = fw;
		rightToIdleAnim->frameHeight = fh;
		rightToIdleAnim->loop = false;
		rightToIdleAnim->transitionsOnlyAtEnd = false;
		rightToIdleAnim->frames.push_back({ 5, 0, 0.1f });  // Frame 6 (col 5)
		rightToIdleAnim->frames.push_back({ 4, 0, 0.1f });  // Frame 5 (col 4)
		rightToIdleAnim->frames.push_back({ 3, 0, 0.1f });  // Frame 4 (col 3)
		animator->graph->AddState("rightToIdle", std::move(rightToIdleAnim));

		// Set starting state
		animator->graph->startState = "idle";

		// Add transitions (initially without conditions)
		animator->graph->AddTransition("idle", "moveLeft", std::function<bool()>());
		animator->graph->AddTransition("idle", "moveRight", std::function<bool()>());
		animator->graph->AddTransition("moveLeft", "leftToIdle", std::function<bool()>());
		animator->graph->AddTransition("moveLeft", "moveRight", std::function<bool()>());
		animator->graph->AddTransition("moveRight", "rightToIdle", std::function<bool()>());
		animator->graph->AddTransition("moveRight", "moveLeft", std::function<bool()>());
		animator->graph->AddTransition("leftToIdle", "idle", std::function<bool()>());
		animator->graph->AddTransition("leftToIdle", "moveLeft", std::function<bool()>());
		animator->graph->AddTransition("leftToIdle", "moveRight", std::function<bool()>());
		animator->graph->AddTransition("rightToIdle", "idle", std::function<bool()>());
		animator->graph->AddTransition("rightToIdle", "moveLeft", std::function<bool()>());
		animator->graph->AddTransition("rightToIdle", "moveRight", std::function<bool()>());

		// Set conditions
		animator->graph->SetTransitionCondition("idle", "moveLeft", [this]() { return movement.x < 0.f; });
		animator->graph->SetTransitionCondition("idle", "moveRight", [this]() { return movement.x > 0.f; });
		animator->graph->SetTransitionCondition("moveLeft", "leftToIdle", [this]() { return movement.x == 0.0f; });
		animator->graph->SetTransitionCondition("moveLeft", "moveRight", [this]() { return movement.x > 0.f; });
		animator->graph->SetTransitionCondition("moveRight", "rightToIdle", [this]() { return movement.x == 0.0f; });
		animator->graph->SetTransitionCondition("moveRight", "moveLeft", [this]() { return movement.x < 0.f; });
		animator->graph->SetTransitionCondition("leftToIdle", "idle", []() { return true; });  // Always transition
		animator->graph->SetTransitionCondition("leftToIdle", "moveLeft", [this]() { return movement.x < 0.f; });
		animator->graph->SetTransitionCondition("leftToIdle", "moveRight", [this]() { return movement.x > 0.f; });
		animator->graph->SetTransitionCondition("rightToIdle", "idle", []() { return true; });  // Always transition
		animator->graph->SetTransitionCondition("rightToIdle", "moveLeft", [this]() { return movement.x < 0.f; });
		animator->graph->SetTransitionCondition("rightToIdle", "moveRight", [this]() { return movement.x > 0.f; });


		animator->Start();
		animator->SetActive(true);
	}



	void OnUpdate(float deltaTime) override
	{
		
		// Update code for SpaceShip
		movement = Vec2::Zero();
		
		// read input and move the spaceship
		
		if (Input::GetGamepadAxis(GamepadAxis::LeftX) != 0 || Input::GetGamepadAxis(GamepadAxis::LeftY) != 0)
		{
			movement.x = Input::GetGamepadAxis(GamepadAxis::LeftX);
			movement.y = Input::GetGamepadAxis(GamepadAxis::LeftY);
		}
		else {
			if (Input::IsKeyDown(Key::A))// || Input::IsGamepadButtonDown(GamepadButton::DPadLeft))
			{
				movement.x = -1;
			}
			if (Input::IsKeyDown(Key::D))// || Input::IsGamepadButtonDown(GamepadButton::DPadRight))
			{
				movement.x = 1;
			}
			if (Input::IsKeyDown(Key::W))// || Input::IsGamepadButtonDown(GamepadButton::DPadUp))
			{
				movement.y = -1;
			}
			if (Input::IsKeyDown(Key::S))// || Input::IsGamepadButtonDown(GamepadButton::DPadDown))
			{
				movement.y = 1;
			}
		}
		
		
		movement = movement.Normalize() * deltaTime;
		movement.x *= speedX;
		movement.y *= speedY;
		tr->MoveLocal(movement.x, movement.y);
	}

	void OnDestroy() override
	{
		// Cleanup code for SpaceShip
	}

private:

};

#pragma endregion





#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include "Component.h"
#include "Texture.h"
#include "FRect.hpp"
#include "SpriteRenderer.h"

#include "GameObject.h"

// Represents a single frame in an animation (modified to use grid-based row/column instead of explicit clipRect)
struct AnimationFrame {
	int col = 0;  // Column index (0-based)
	int row = 0;  // Row index (0-based, default 0 for single-row spritesheets)
	float duration = 0.1f;  // Duration in seconds
};

// Represents an animation sequence (modified to include texture, frame width/height for grid computation)
class Animation {
public:
	std::string name;
	Texture* texture = nullptr;  // Texture for the animation (if null, use SpriteRenderer's current texture)
	float frameWidth = 0.f;      // Width of each frame in the grid
	float frameHeight = 0.f;     // Height of each frame in the grid
	std::vector<AnimationFrame> frames;
	bool loop = false;
	bool transitionsOnlyAtEnd = false;  // If true, transitions are only checked when the animation reaches the last frame
};

// Represents a transition between animation states
struct Transition {
	std::string to;
	std::function<bool()> condition;
};

// Represents a state in the animation graph
class AnimationState {
public:
	std::unique_ptr<Animation> animation;
	std::vector<Transition> transitions;
};

// Represents the entire animation graph
class AnimationGraph {
public:
	std::unordered_map<std::string, AnimationState> states;
	std::string startState;

	// Adds a state to the graph
	void AddState(const std::string& name, std::unique_ptr<Animation> anim) {
		states[name].animation = std::move(anim);
	}

	// Adds a transition between states
	void AddTransition(const std::string& from, const std::string& to, std::function<bool()> cond) {
		states[from].transitions.push_back({ to, std::move(cond) });
	}

	// Sets or updates the condition for a specific transition (by from and to)
	void SetTransitionCondition(const std::string& from, const std::string& to, std::function<bool()> cond) {
		auto it = states.find(from);
		if (it != states.end()) {
			for (auto& trans : it->second.transitions) {
				if (trans.to == to) {
					trans.condition = std::move(cond);
					return;
				}
			}
		}
		// If not found, you could add it here if desired, but for now, do nothing
	}
};

// Component that plays animations based on the AnimationGraph
class Animator : public Component {
public:
	Animator() { name = "Animator " + std::to_string(GetId()); }
	std::unique_ptr<AnimationGraph> graph;
	std::string currentState;

	// Initialize the animator
	void Start() override {
		if (graph && !graph->startState.empty()) {
			currentState = graph->startState;
			currentTime = 0.f;
			currentFrame = 0;
		}
	}

	// Update the animator each frame
	void Update(float deltatime) override {
		if (!graph || graph->states.find(currentState) == graph->states.end()) return;

		std::cout << "Animator Update: Current State = " << currentState << ", Current Frame = " << currentFrame << ", Current Time = " << currentTime << std::endl;
		auto& state = graph->states[currentState];
		auto* anim = state.animation.get();
		if (!anim || anim->frames.empty()) return;

		currentTime += deltatime;

		float frameDur = anim->frames[currentFrame].duration;

		while (currentTime >= frameDur) {
			currentTime -= frameDur;
			currentFrame++;

			if (currentFrame >= anim->frames.size()) {
				if (anim->loop) {
					currentFrame = 0;
				}
				else {
					currentFrame = anim->frames.size() - 1;
					break;
				}
			}

			frameDur = anim->frames[currentFrame].duration;
		}

		// Apply current frame to SpriteRenderer (compute clipRect from row, col, frameWidth, frameHeight)
		auto* sprite = owner->GetComponent<SpriteRenderer>();
		if (sprite) {
			const auto& frame = anim->frames[currentFrame];
			float x = static_cast<float>(frame.col) * anim->frameWidth;
			float y = static_cast<float>(frame.row) * anim->frameHeight;
			if (anim->texture) sprite->texture = anim->texture;
			sprite->clipRect = { x, y, anim->frameWidth, anim->frameHeight };
		}

		// Check transitions only if allowed (based on transitionsOnlyAtEnd)
		if (!anim->transitionsOnlyAtEnd || (currentFrame == anim->frames.size() - 1)) {
			for (const auto& trans : state.transitions) {
				if (trans.condition && trans.condition()) {
					currentState = trans.to;
					currentTime = 0.f;
					currentFrame = 0;
					break;  // Apply first matching transition
				}
			}
		}
	}

private:
	float currentTime = 0.f;
	size_t currentFrame = 0;
};
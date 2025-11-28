// Add AnimationSystem.h (new file for animations and graph)

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

// Represents a single frame in an animation
struct AnimationFrame {
	Texture* texture = nullptr;  // Optional; if null, use SpriteRenderer's current texture
	FRect clipRect = { 0.f, 0.f, 0.f, 0.f };  // Frame region; if w <= 0, use full
	float duration = 0.1f;  // Default duration in seconds
};

// Represents an animation sequence
class Animation {
public:
	std::string name;
	std::vector<AnimationFrame> frames;
	bool loop = false;
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
};

// Component that plays animations based on the AnimationGraph
class Animator : public Component {
public:
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

		auto& state = graph->states[currentState];
		auto* anim = state.animation.get();
		if (!anim || anim->frames.empty()) return;

		currentTime += deltatime;

		float frameDur = anim->frames[currentFrame].duration;
		bool reachedEnd = false;

		while (currentTime >= frameDur) {
			currentTime -= frameDur;
			currentFrame++;

			if (currentFrame >= anim->frames.size()) {
				reachedEnd = true;
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

		// Apply current frame to SpriteRenderer
		auto* sprite = owner->GetComponent<SpriteRenderer>();
		if (sprite) {
			const auto& frame = anim->frames[currentFrame];
			if (frame.texture) sprite->texture = frame.texture;
			sprite->clipRect = frame.clipRect;
		}

		// Check transitions (always check, even if not at end)
		for (const auto& trans : state.transitions) {
			if (trans.condition && trans.condition()) {
				currentState = trans.to;
				currentTime = 0.f;
				currentFrame = 0;
				break;  // Apply first matching transition
			}
		}
	}

private:
	float currentTime = 0.f;
	size_t currentFrame = 0;
};


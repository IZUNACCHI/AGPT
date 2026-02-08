#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "MonoBehaviour.h"
#include "Time.hpp"

#include "AnimatorController.h"
#include "AnimationClip.h"

class SpriteRenderer;

// Animator component: plays back AnimationClips based on an AnimatorController graph.
class Animator final : public MonoBehaviour {
public:
	Animator();
	~Animator() override = default;

	void Awake() override;
	void Update() override;

	/// Assign a controller (graph asset). Resets params to defaults if missing,
	/// and enters the controller's entry state.
	void SetController(AnimatorController* controller);
	AnimatorController* GetController() const { return m_controller; }

	// Parameter API
	void SetFloat(const std::string& name, float v);
	void SetInt(const std::string& name, int v);
	void SetBool(const std::string& name, bool v);
	void SetTrigger(const std::string& name);

	float GetFloat(const std::string& name) const;
	int GetInt(const std::string& name) const;
	bool GetBool(const std::string& name) const;
	bool GetTrigger(const std::string& name) const;

	/// Force a state by name (ignores transition rules).
	void Play(const std::string& stateName, bool restart = true);

	std::string GetCurrentStateName() const;
	float GetStateTime() const { return m_stateTime; }

	/// Scrub the current state's clip toward a normalized target [0..1] at the given speed
	/// (normalized units per second). For condition based frames like turning the ship
	void SeekNormalized(float targetN, float speedNormalizedPerSec);

	std::shared_ptr<Component> Clone() const override;

private:

	// Ensure all parameters have values, using controller defaults if needed.
	void EnsureDefaultsFromController();
	// Evaluate transitions from the current state, and apply the first valid one.
	void EvaluateAndApplyTransitions();
	// Check if a transition can be taken.
	bool CanTakeTransition(const AnimTransition& tr) const;
	// Check if the conditions for a transition are met.
	bool ConditionsMet(const AnimTransition& tr) const;
	// Check if the exit time condition for a transition is met.
	bool ExitTimeMet(const AnimTransition& tr) const;

	// Switch to a new state by ID.
	void SwitchState(int newStateId, bool restartTime);
	const AnimState* CurrentState() const;

	void ApplyCurrentClipFrame();

	void ConsumeTriggersUsedBy(const AnimTransition& tr);
	void ClearAllTriggers();

private:
	AnimatorController* m_controller = nullptr;

	std::unordered_map<std::string, float> m_floats;
	std::unordered_map<std::string, int> m_ints;
	std::unordered_map<std::string, bool> m_bools;
	std::unordered_map<std::string, bool> m_triggers;

	int m_stateId = -1; // Current state ID
	float m_stateTime = 0.0f; // Time spent in current state

	SpriteRenderer* m_sprite = nullptr; // Cached SpriteRenderer
	Texture* m_lastTexture = nullptr; // To detect texture changes
	Vector2i m_lastFrameSize = Vector2i::Zero(); // To detect frame size changes

	bool m_timeOverriddenThisFrame = false; // True if SeekNormalized was called this frame

	int m_prevLocalFrame = -1; // To avoid redundant frame sets
};

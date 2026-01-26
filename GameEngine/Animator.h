#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "MonoBehaviour.h"
#include "Time.hpp"

#include "AnimatorController.h"
#include "AnimationClip.h"

class SpriteRenderer;

/// Runtime animator that plays an AnimatorController on a SpriteRenderer.
///
/// Features:
/// - states with sprite clips
/// - transitions with conditions (bool/int/float/trigger)
/// - Any-State transitions
/// - optional exit-time transitions
/// - triggers are one-frame pulses
///
/// No blending, no layers.
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
	/// (normalized units per second). Great for "turn left/right then return to center".
	void SeekNormalized(float targetN, float speedNormalizedPerSec);

	std::shared_ptr<Component> Clone() const override;

private:
	void EnsureDefaultsFromController();
	void EvaluateAndApplyTransitions();
	bool CanTakeTransition(const AnimTransition& tr) const;
	bool ConditionsMet(const AnimTransition& tr) const;
	bool ExitTimeMet(const AnimTransition& tr) const;

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

	int m_stateId = -1;
	float m_stateTime = 0.0f;

	SpriteRenderer* m_sprite = nullptr;
	Texture* m_lastTexture = nullptr;
	Vector2i m_lastFrameSize = Vector2i::Zero();

	int m_prevLocalFrame = -1;
};

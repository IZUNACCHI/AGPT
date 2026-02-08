#pragma once

#include <string>
#include <vector>

class AnimationClip;

/// Parameter types used by AnimatorController.
enum class AnimParamType { Float, Int, Bool, Trigger };

struct AnimParamDef {
	std::string name; // Unique name.
	AnimParamType type = AnimParamType::Float; // Default type.
	float defaultFloat = 0.0f; // Used if type == Float
	int defaultInt = 0; // Used if type == Int
	bool defaultBool = false; // Used if type == Bool
};

/// Condition operators for transitions.
enum class AnimCondOp {
	BoolTrue,
	BoolFalse,
	FloatGreater,
	FloatLess,
	FloatGreaterEq,
	FloatLessEq,
	IntEquals,
	IntNotEquals,
	TriggerSet
};

// Condition for an animation transition.
struct AnimCondition {
	std::string param;
	AnimCondOp op = AnimCondOp::BoolTrue;
	float f = 0.0f;
	int i = 0;
};

// Transition between two animation states.
struct AnimTransition {
	// If fromState == -1 => Any State transition.
	int fromState = -1;
	int toState = -1;

	// If true, transition can only occur after the fromState's clip has played for exitTimeNormalized portion.
	bool hasExitTime = false;
	float exitTimeNormalized = 1.0f; // 0..1

	/// All conditions must pass.
	std::vector<AnimCondition> conditions;
};

// Animation state: id + name + clip.
struct AnimState {
	int id = -1;
	std::string name;
	AnimationClip* clip = nullptr;
};

// Immutable graph asset: states + transitions + parameter defaults.
class AnimatorController {
public:
	// Parameter definitions.
	std::vector<AnimParamDef> parameters;
	// Animation states and transitions.
	std::vector<AnimState> states;
	std::vector<AnimTransition> transitions;
	int entryState = -1;

	// Find state by its unique id. Returns nullptr if not found.
	const AnimState* FindStateById(int id) const {
		for (const auto& s : states) {
			if (s.id == id) return &s;
		}
		return nullptr;
	}

	// Find state id by its unique name. Returns -1 if not found.
	int FindStateIdByName(const std::string& n) const {
		for (const auto& s : states) {
			if (s.name == n) return s.id;
		}
		return -1;
	}
};

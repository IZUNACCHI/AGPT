#pragma once

#include <string>
#include <vector>

class AnimationClip;

/// Parameter types used by AnimatorController.
enum class AnimParamType { Float, Int, Bool, Trigger };

struct AnimParamDef {
	std::string name;
	AnimParamType type = AnimParamType::Float;
	float defaultFloat = 0.0f;
	int defaultInt = 0;
	bool defaultBool = false;
};

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

struct AnimCondition {
	std::string param;
	AnimCondOp op = AnimCondOp::BoolTrue;
	float f = 0.0f;
	int i = 0;
};

struct AnimTransition {
	/// If fromState == -1 => Any State transition.
	int fromState = -1;
	int toState = -1;

	/// Optional: require current clip to reach this normalized time.
	bool hasExitTime = false;
	float exitTimeNormalized = 1.0f; // 0..1

	/// All conditions must pass.
	std::vector<AnimCondition> conditions;
};

struct AnimState {
	int id = -1;
	std::string name;
	AnimationClip* clip = nullptr;
};

/// Immutable graph asset: states + transitions + parameter defaults.
class AnimatorController {
public:
	std::vector<AnimParamDef> parameters;
	std::vector<AnimState> states;
	std::vector<AnimTransition> transitions;
	int entryState = -1;

	const AnimState* FindStateById(int id) const {
		for (const auto& s : states) {
			if (s.id == id) return &s;
		}
		return nullptr;
	}

	int FindStateIdByName(const std::string& n) const {
		for (const auto& s : states) {
			if (s.name == n) return s.id;
		}
		return -1;
	}
};

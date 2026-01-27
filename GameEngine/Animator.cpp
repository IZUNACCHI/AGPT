#include "Animator.h"
#include "SpriteRenderer.h"
#include "GameObject.h"

#include <algorithm>

Animator::Animator()
	: MonoBehaviour() {
	// Give the component a distinctive name (MonoBehaviour constructor sets "MonoBehaviour").
	SetComponentName("Animator");
}

void Animator::Awake() {
	// Cache the SpriteRenderer if present.
	auto sprite = GetComponent<SpriteRenderer>();
	m_sprite = sprite ? sprite.get() : nullptr;
}

void Animator::SetController(AnimatorController* controller) {
	m_controller = controller;
	EnsureDefaultsFromController();

	if (m_controller && m_controller->entryState != -1) {
		SwitchState(m_controller->entryState, true);
	}
}

// ---------------- Params ----------------

void Animator::SetFloat(const std::string& name, float v) { m_floats[name] = v; }
void Animator::SetInt(const std::string& name, int v) { m_ints[name] = v; }
void Animator::SetBool(const std::string& name, bool v) { m_bools[name] = v; }
void Animator::SetTrigger(const std::string& name) { m_triggers[name] = true; }

float Animator::GetFloat(const std::string& name) const {
	auto it = m_floats.find(name);
	return it != m_floats.end() ? it->second : 0.0f;
}

int Animator::GetInt(const std::string& name) const {
	auto it = m_ints.find(name);
	return it != m_ints.end() ? it->second : 0;
}

bool Animator::GetBool(const std::string& name) const {
	auto it = m_bools.find(name);
	return it != m_bools.end() ? it->second : false;
}

bool Animator::GetTrigger(const std::string& name) const {
	auto it = m_triggers.find(name);
	return it != m_triggers.end() ? it->second : false;
}

void Animator::EnsureDefaultsFromController() {
	if (!m_controller) return;

	for (const auto& p : m_controller->parameters) {
		switch (p.type) {
			case AnimParamType::Float:
				if (m_floats.find(p.name) == m_floats.end()) m_floats[p.name] = p.defaultFloat;
				break;
			case AnimParamType::Int:
				if (m_ints.find(p.name) == m_ints.end()) m_ints[p.name] = p.defaultInt;
				break;
			case AnimParamType::Bool:
				if (m_bools.find(p.name) == m_bools.end()) m_bools[p.name] = p.defaultBool;
				break;
			case AnimParamType::Trigger:
				if (m_triggers.find(p.name) == m_triggers.end()) m_triggers[p.name] = false;
				break;
		}
	}
}

// ---------------- State control ----------------

void Animator::Play(const std::string& stateName, bool restart) {
	if (!m_controller) return;
	int id = m_controller->FindStateIdByName(stateName);
	if (id != -1) {
		SwitchState(id, restart);
	}
}

std::string Animator::GetCurrentStateName() const {
	const AnimState* s = CurrentState();
	return s ? s->name : std::string();
}

const AnimState* Animator::CurrentState() const {
	if (!m_controller) return nullptr;
	return m_controller->FindStateById(m_stateId);
}

void Animator::SwitchState(int newStateId, bool restartTime) {
	if (newStateId == -1) return;
	if (m_stateId == newStateId && !restartTime) return;

	m_stateId = newStateId;
	if (restartTime) {
		m_stateTime = 0.0f;
		m_prevLocalFrame = -1;
	}
}

// ---------------- Optional Seek ----------------

void Animator::SeekNormalized(float targetN, float speedNormalizedPerSec) {
	const AnimState* s = CurrentState();
	if (!s || !s->clip) return;

	AnimationClip* clip = s->clip;
	const float len = clip->GetLengthSeconds();
	if (len <= 0.0f) return;

	targetN = std::clamp(targetN, 0.0f, 1.0f);
	const float curN = clip->GetNormalizedTime(m_stateTime);

	const float dt = Time::DeltaTime();
	const float step = std::max(0.0f, speedNormalizedPerSec) * dt;

	float newN = curN;
	if (curN < targetN) newN = std::min(curN + step, targetN);
	else if (curN > targetN) newN = std::max(curN - step, targetN);

	m_stateTime = newN * len;
}

// ---------------- Update loop ----------------

void Animator::Update() {
	if (!m_controller) {
		ClearAllTriggers();
		return;
	}

	// SpriteRenderer might be added after Awake
	if (!m_sprite) {
		auto sprite = GetComponent<SpriteRenderer>();
		m_sprite = sprite ? sprite.get() : nullptr;
	}

	// Advance time
	m_stateTime += Time::DeltaTime();

	// Apply transitions
	EvaluateAndApplyTransitions();

	// Apply current clip to SpriteRenderer
	ApplyCurrentClipFrame();

	// Triggers are one-frame pulses
	ClearAllTriggers();
}

void Animator::EvaluateAndApplyTransitions() {
	if (!m_controller) return;

	// Any State transitions first (in list order)
	for (const auto& tr : m_controller->transitions) {
		if (tr.fromState != -1) continue;
		if (CanTakeTransition(tr)) {
			ConsumeTriggersUsedBy(tr);
			SwitchState(tr.toState, true);
			return;
		}
	}

	// Then transitions from current state
	for (const auto& tr : m_controller->transitions) {
		if (tr.fromState != m_stateId) continue;
		if (CanTakeTransition(tr)) {
			ConsumeTriggersUsedBy(tr);
			SwitchState(tr.toState, true);
			return;
		}
	}
}

bool Animator::CanTakeTransition(const AnimTransition& tr) const {
	if (tr.toState == -1) return false;
	if (tr.hasExitTime && !ExitTimeMet(tr)) return false;
	if (!ConditionsMet(tr)) return false;
	return true;
}

bool Animator::ExitTimeMet(const AnimTransition& tr) const {
	const AnimState* s = CurrentState();
	if (!s || !s->clip) return true;

	const float n = s->clip->GetNormalizedTime(m_stateTime);
	return n >= tr.exitTimeNormalized;
}

bool Animator::ConditionsMet(const AnimTransition& tr) const {
	for (const auto& c : tr.conditions) {
		switch (c.op) {
			case AnimCondOp::BoolTrue:
				if (!GetBool(c.param)) return false;
				break;
			case AnimCondOp::BoolFalse:
				if (GetBool(c.param)) return false;
				break;
			case AnimCondOp::FloatGreater:
				if (!(GetFloat(c.param) > c.f)) return false;
				break;
			case AnimCondOp::FloatLess:
				if (!(GetFloat(c.param) < c.f)) return false;
				break;
			case AnimCondOp::FloatGreaterEq:
				if (!(GetFloat(c.param) >= c.f)) return false;
				break;
			case AnimCondOp::FloatLessEq:
				if (!(GetFloat(c.param) <= c.f)) return false;
				break;
			case AnimCondOp::IntEquals:
				if (!(GetInt(c.param) == c.i)) return false;
				break;
			case AnimCondOp::IntNotEquals:
				if (!(GetInt(c.param) != c.i)) return false;
				break;
			case AnimCondOp::TriggerSet:
				if (!GetTrigger(c.param)) return false;
				break;
		}
	}
	return true;
}

void Animator::ConsumeTriggersUsedBy(const AnimTransition& tr) {
	for (const auto& c : tr.conditions) {
		if (c.op == AnimCondOp::TriggerSet) {
			auto it = m_triggers.find(c.param);
			if (it != m_triggers.end()) it->second = false;
		}
	}
}

void Animator::ClearAllTriggers() {
	for (auto& kv : m_triggers) {
		kv.second = false;
	}
}

// ---------------- Apply ----------------

void Animator::ApplyCurrentClipFrame() {
	if (!m_sprite) return;

	const AnimState* s = CurrentState();
	if (!s || !s->clip) return;

	AnimationClip* clip = s->clip;
	if (!clip->IsValid()) return;

	// Ensure SpriteRenderer is using the clip's spritesheet.
	Texture* desiredTexture = clip->sheet->texture;
	Vector2i desiredFrameSize = clip->sheet->frameSize;

	if (desiredTexture != m_lastTexture) {
		m_sprite->SetTexture(desiredTexture);
		m_lastTexture = desiredTexture;
	}
	if (desiredFrameSize != m_lastFrameSize) {
		m_sprite->SetFrameSize(desiredFrameSize);
		m_lastFrameSize = desiredFrameSize;
	}

	// Sample frame index and apply
	const int frameIndex = clip->SampleFrameIndex(m_stateTime);
	if (frameIndex >= 0) {
		m_sprite->SetFrameIndex(frameIndex);
	}

	// Optional: fire frame events when entering a new local frame.
	const int localFrame = clip->SampleLocalFrame(m_stateTime);
	if (localFrame != -1 && localFrame != m_prevLocalFrame) {
		m_prevLocalFrame = localFrame;
		for (const auto& ev : clip->events) {
			if (ev.localFrameIndex == localFrame) {
				// Hook into your existing message system if desired.
				// Example:
				// ReceiveMessage(ev.name);
			}
		}
	}
}

std::shared_ptr<Component> Animator::Clone() const {
	auto clone = std::make_shared<Animator>();
	clone->m_controller = m_controller;
	clone->m_floats = m_floats;
	clone->m_ints = m_ints;
	clone->m_bools = m_bools;
	clone->m_triggers = m_triggers;
	clone->m_stateId = m_stateId;
	clone->m_stateTime = m_stateTime;
	return clone;
}

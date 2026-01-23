#include "MonoBehaviour.h"
#include "GameObject.h"
#include "Scene.h"
#include "Time.hpp"
#include <algorithm>
#include <utility>

MonoBehaviour::MonoBehaviour()
	: Behaviour("MonoBehaviour") {
}

// Generates a new invoke handle.
MonoBehaviour::InvokeHandle MonoBehaviour::MakeInvokeId() {
	return m_nextInvokeId++;
}


// Schedules a function to run after a delay.
MonoBehaviour::InvokeHandle MonoBehaviour::Invoke(
	std::function<void()> fn,
	float delaySeconds,
	InvokeTickPolicy policy) {

	if (!fn) return 0;

	InvokeRequest req;
	req.id = MakeInvokeId();
	req.fn = std::move(fn);
	req.policy = policy;
	req.nextTime = Time::Now() + (std::max)(0.0f, delaySeconds);

	// If ticking depends on enabled state and we're disabled, start paused.
	if (policy == InvokeTickPolicy::WhileBehaviourEnabled && !IsActiveAndEnabled()) {
		req.paused = true;
		req.pausedRemaining = (std::max)(0.0f, req.nextTime - Time::Now());
	}

	m_invokes.push_back(std::move(req));
	return m_invokes.back().id;
}

// Schedules a function to run repeatedly.
MonoBehaviour::InvokeHandle MonoBehaviour::InvokeRepeating(
	std::function<void()> fn,
	float delaySeconds,
	float rateSeconds,
	InvokeTickPolicy policy) {

	if (!fn) return 0;

	InvokeRequest req;
	req.id = MakeInvokeId();
	req.fn = std::move(fn);
	req.policy = policy;
	req.nextTime = Time::Now() + (std::max)(0.0f, delaySeconds);
	req.rate = (std::max)(0.0f, rateSeconds);
	req.repeating = true;

	// If ticking depends on enabled state and we're disabled, start paused.
	if (policy == InvokeTickPolicy::WhileBehaviourEnabled && !IsActiveAndEnabled()) {
		req.paused = true;
		req.pausedRemaining = (std::max)(0.0f, req.nextTime - Time::Now());
	}

	m_invokes.push_back(std::move(req));
	return m_invokes.back().id;
}

// Cancels a scheduled invoke.
void MonoBehaviour::CancelInvoke(InvokeHandle handle) {
	if (handle == 0) return;
	for (auto& r : m_invokes) {
		if (r.id == handle) {
			r.cancelled = true;
			break;
		}
	}
}

// Returns true if the handle is still scheduled.
bool MonoBehaviour::IsInvoking(InvokeHandle handle) const {
	if (handle == 0) return false;
	for (const auto& r : m_invokes) {
		if (!r.cancelled && r.id == handle) return true;
	}
	return false;
}

// Pauses a scheduled invoke without removing it.
bool MonoBehaviour::PauseInvoke(InvokeHandle handle) {
	if (handle == 0) return false;
	const float now = Time::Now();

	for (auto& r : m_invokes) {
		if (r.cancelled || r.id != handle) continue;

		if (!r.paused) {
			r.paused = true;
			r.pausedRemaining = (std::max)(0.0f, r.nextTime - now);
		}
		return true;
	}
	return false;
}

// Resumes a paused invoke.
bool MonoBehaviour::ResumeInvoke(InvokeHandle handle) {
	if (handle == 0) return false;
	const float now = Time::Now();

	for (auto& r : m_invokes) {
		// Skip cancelled or non-matching
		if (r.cancelled || r.id != handle) continue;

		// Resume if paused
		if (r.paused) {
			r.paused = false;
			r.nextTime = now + (std::max)(0.0f, r.pausedRemaining);
			r.pausedRemaining = 0.0f;
		}
		return true;
	}
	return false;
}

// Restarts the invoke countdown.
bool MonoBehaviour::RestartInvoke(InvokeHandle handle) {
	if (handle == 0) return false;
	const float now = Time::Now();

	for (auto& r : m_invokes) {
		// Skip cancelled or non-matching
		if (r.cancelled || r.id != handle) continue;

		// Calculate remaining time, continue from paused time if paused
		const float remaining = r.paused
			? (std::max)(0.0f, r.pausedRemaining)
			: (std::max)(0.0f, r.nextTime - now);

		r.paused = false;
		r.pausedRemaining = 0.0f;
		r.nextTime = now + remaining;
		return true;
	}
	return false;
}

// Pauses all scheduled invokes on this behaviour.
void MonoBehaviour::PauseAllInvokes() {
	const float now = Time::Now();
	for (auto& r : m_invokes) {
		if (r.cancelled) continue;
		if (!r.paused) {
			r.paused = true;
			r.pausedRemaining = (std::max)(0.0f, r.nextTime - now);
		}
	}
}

// Resumes all paused invokes on this behaviour.
void MonoBehaviour::ResumeAllInvokes() {
	const float now = Time::Now();
	for (auto& r : m_invokes) {
		if (r.cancelled) continue;
		if (r.paused) {
			r.paused = false;
			r.nextTime = now + (std::max)(0.0f, r.pausedRemaining);
			r.pausedRemaining = 0.0f;
		}
	}
}

// String based invokes

// Schedules a method by name after a delay.
void MonoBehaviour::Invoke(const std::string& methodName, float time) {
	if (methodName.empty()) return;

	InvokeRequest req;
	req.id = MakeInvokeId();
	req.methodName = methodName;
	req.policy = InvokeTickPolicy::WhileGameObjectActive;
	req.nextTime = Time::Now() + (std::max)(0.0f, time);

	m_invokes.push_back(std::move(req));
}

// Schedules a method by name repeatedly.
void MonoBehaviour::InvokeRepeating(const std::string& methodName, float delay, float rate) {
	if (methodName.empty()) return;

	InvokeRequest req;
	req.id = MakeInvokeId();
	req.methodName = methodName;
	req.policy = InvokeTickPolicy::WhileGameObjectActive;
	req.nextTime = Time::Now() + (std::max)(0.0f, delay);
	req.rate = (std::max)(0.0f, rate);
	req.repeating = true;

	m_invokes.push_back(std::move(req));
}


// Cancels scheduled invokes by name (or all if empty).
void MonoBehaviour::CancelInvoke(const std::string& methodName) {
	if (methodName.empty()) {
		m_invokes.clear();
		return;
	}

	m_invokes.erase(
		std::remove_if(m_invokes.begin(), m_invokes.end(),
			[&](const InvokeRequest& r) { return r.methodName == methodName; }),
		m_invokes.end());
}

// Returns true if any invokes with the given name are scheduled.
bool MonoBehaviour::IsInvoking(const std::string& methodName) const {
	for (const auto& r : m_invokes) {
		if (!r.cancelled && r.methodName == methodName) return true;
	}
	return false;
}

// Registers a method name to function handler.
void MonoBehaviour::RegisterInvokeHandler(const std::string& methodName, std::function<void()> handler) {
	if (methodName.empty()) return;
	m_invokeHandlers[methodName] = std::move(handler);
}

// Unregisters a method name handler.
void MonoBehaviour::UnregisterInvokeHandler(const std::string& methodName) {
	if (methodName.empty()) return;
	m_invokeHandlers.erase(methodName);
}

// Receives a message to invoke a method by name.
void MonoBehaviour::ReceiveMessage(const std::string& methodName) {
	auto it = m_invokeHandlers.find(methodName);
	if (it != m_invokeHandlers.end() && it->second) {
		it->second();
	}
}

// InvokerStateHandling

// Returns whether OnEnable has been called.
bool MonoBehaviour::HasOnEnableBeenCalled() const {
	return m_onEnableCalled;
}

// Applies invoke pausing rules when enabled state changes.
void MonoBehaviour::OnBehaviourEnabledChangedForInvokes(bool enabled) {
	const float now = Time::Now();

	for (auto& r : m_invokes) {
		if (r.cancelled) continue;
		if (r.policy != InvokeTickPolicy::WhileBehaviourEnabled) continue;

		if (!enabled) {
			if (!r.paused) {
				r.paused = true;
				r.pausedRemaining = (std::max)(0.0f, r.nextTime - now);
			}
		}
		else {
			if (r.paused) {
				r.paused = false;
				r.nextTime = now + (std::max)(0.0f, r.pausedRemaining);
				r.pausedRemaining = 0.0f;
			}
		}
	}
}

// Called when the enabled flag changes.
void MonoBehaviour::OnEnabledStateChanged(bool enabled) {
	if (!m_gameObject || !m_gameObject->IsActiveInHierarchy()) return;

	OnBehaviourEnabledChangedForInvokes(enabled);

	if (enabled) {
		if (auto* scene = m_gameObject->GetScene()) {
			scene->QueueLifecycle(this);
		}
	}
	else {
		TriggerDisable();
	}
}

// Lifecycle triggers

void MonoBehaviour::TriggerAwake() {
	if (m_didAwake) return;
	m_didAwake = true;
	Awake();
}

void MonoBehaviour::TriggerEnable() {
	if (m_onEnableCalled) return;
	m_onEnableCalled = true;
	m_hasEverBeenActive = true;
	OnEnable();
}

void MonoBehaviour::TriggerDisable() {
	if (!m_onEnableCalled) return;
	m_onEnableCalled = false;
	OnDisable();
}

void MonoBehaviour::TriggerStart() {
	if (m_didStart) return;
	m_didStart = true;
	Start();
}

void MonoBehaviour::TriggerDestroy() {
	if (m_destroyCallbacksSent) return;
	m_destroyCallbacksSent = true;

	if (m_onEnableCalled) {
		TriggerDisable();
	}

	if (m_hasEverBeenActive) {
		OnDestroy();
	}
}

// Invoke ticking

// Advances repeating invokes without accumulating drift.
void MonoBehaviour::AdvanceRepeating(InvokeRequest& r, float now) {
	if (r.rate <= 0.0f) {
		r.nextTime = now;
		return;
	}

	while (r.nextTime <= now) {
		r.nextTime += r.rate;
	}
}

// Executes due invokes.
void MonoBehaviour::TickInvokes(float now) {
	if (m_invokes.empty()) return;

	for (auto& r : m_invokes) {
		if (r.cancelled || r.paused) continue;

		if (r.nextTime <= now) {
			if (r.fn) {
				r.fn();
			}
			else if (!r.methodName.empty()) {
				ReceiveMessage(r.methodName);
			}

			if (r.repeating && !r.cancelled) {
				AdvanceRepeating(r, now);
			}
			else {
				r.cancelled = true;
			}
		}
	}

	m_invokes.erase(
		std::remove_if(m_invokes.begin(), m_invokes.end(),
			[](const InvokeRequest& r) { return r.cancelled; }),
		m_invokes.end());
}
#include "MonoBehaviour.h"
#include "GameObject.h"
#include "Scene.h"
#include "Time.hpp"
#include <algorithm>

MonoBehaviour::MonoBehaviour()
	: Behaviour("MonoBehaviour") {
}

void MonoBehaviour::Invoke(const std::string& methodName, float time) {
	if (methodName.empty()) {
		return;
	}
	InvokeRequest request;
	request.methodName = methodName;
	request.nextTime = Time::Now() + (std::max)(0.0f, time);
	request.rate = 0.0f;
	request.repeating = false;
	m_invokes.push_back(request);
}

void MonoBehaviour::InvokeRepeating(const std::string& methodName, float delay, float rate) {
	if (methodName.empty()) {
		return;
	}
	InvokeRequest request;
	request.methodName = methodName;
	request.nextTime = Time::Now() + (std::max)(0.0f, delay);
	request.rate = (std::max)(0.0f, rate);
	request.repeating = true;
	m_invokes.push_back(request);
}

void MonoBehaviour::CancelInvoke(const std::string& methodName) {
	if (methodName.empty()) {
		m_invokes.clear();
		return;
	}
	m_invokes.erase(
		std::remove_if(m_invokes.begin(), m_invokes.end(),
			[&](const InvokeRequest& request) { return request.methodName == methodName; }),
		m_invokes.end());
}

bool MonoBehaviour::IsInvoking(const std::string& methodName) const {
	for (const auto& request : m_invokes) {
		if (request.methodName == methodName) {
			return true;
		}
	}
	return false;
}

void MonoBehaviour::ReceiveMessage(const std::string&) {
}

bool MonoBehaviour::HasOnEnableBeenCalled() const {
	return m_onEnableCalled;
}

void MonoBehaviour::OnEnabledStateChanged(bool enabled) {
	if (!m_gameObject || !m_gameObject->IsActiveInHierarchy()) {
		return;
	}

	if (enabled) {
		if (auto* scene = m_gameObject->GetScene()) {
			scene->QueueLifecycle(this);
		}
	}
	else {
		TriggerDisable();
	}
}

void MonoBehaviour::TriggerAwake() {
	if (m_didAwake) {
		return;
	}
	m_didAwake = true;
	Awake();
}

void MonoBehaviour::TriggerEnable() {
	if (m_onEnableCalled) {
		return;
	}
	m_onEnableCalled = true;
	m_hasEverBeenActive = true;
	OnEnable();
}

void MonoBehaviour::TriggerDisable() {
	if (!m_onEnableCalled) {
		return;
	}
	m_onEnableCalled = false;
	OnDisable();
}

void MonoBehaviour::TriggerStart() {
	if (m_didStart) {
		return;
	}
	m_didStart = true;
	Start();
}

void MonoBehaviour::TriggerDestroy() {
	if (m_destroyCallbacksSent) {
		return;
	}
	m_destroyCallbacksSent = true;

	if (m_onEnableCalled) {
		TriggerDisable();
	}

	if (m_hasEverBeenActive) {
		OnDestroy();
	}
}

void MonoBehaviour::TickInvokes(float now) {
	if (m_invokes.empty()) {
		return;
	}

	for (auto& request : m_invokes) {
		if (request.nextTime <= now) {
			ReceiveMessage(request.methodName);
			if (request.repeating) {
				request.nextTime = now + request.rate;
			}
			else {
				request.methodName.clear();
			}
		}
	}

	m_invokes.erase(
		std::remove_if(m_invokes.begin(), m_invokes.end(),
			[](const InvokeRequest& request) { return request.methodName.empty(); }),
		m_invokes.end());
}
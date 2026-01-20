#include "Component.h"

#include "Behaviour.h"
#include "GameObject.h"
#include "Scene.h"

Component::Component(std::string name)
	: Object(std::move(name)) {
}

Scene* Component::GetScene() const {
	return m_gameObject ? m_gameObject->GetScene() : nullptr;
}

bool Component::IsEffectivelyEnabled() const {
	return m_enabled && m_gameObject && m_gameObject->IsActiveInHierarchy() && !IsPendingDestroy();
}

void Component::CallAwakeInit() {
	if (m_awakeCalled) {
		return;
	}
	m_awakeCalled = true;
	Awake();
	Init();
}

void Component::CallOnCreate() {
	if (m_createCalled) {
		return;
	}
	m_createCalled = true;
	OnCreate();
}

void Component::OnActiveInHierarchyChanged(bool nowActive) {
	const bool nowEffective = m_enabled && nowActive && !IsPendingDestroy();
	if (nowEffective == m_effectivelyEnabled) {
		return;
	}

	m_effectivelyEnabled = nowEffective;
	if (m_effectivelyEnabled) {
		OnEnable();
	}
	else {
		OnDisable();
	}

	if (auto* scene = GetScene()) {
		if (auto* behaviour = dynamic_cast<Behaviour*>(this)) {
			if (m_effectivelyEnabled) {
				scene->QueueBehaviourAdd(behaviour);
			}
			else {
				scene->QueueBehaviourRemove(behaviour);
			}
		}
	}
}

void Component::SetEnabled(bool enabled) {
	if (m_enabled == enabled) {
		return;
	}

	const bool wasEffective = IsEffectivelyEnabled();
	m_enabled = enabled;
	const bool nowEffective = IsEffectivelyEnabled();

	if (wasEffective != nowEffective) {
		m_effectivelyEnabled = nowEffective;
		if (m_effectivelyEnabled) {
			OnEnable();
		}
		else {
			OnDisable();
		}
	}

	if (auto* scene = GetScene()) {
		if (auto* behaviour = dynamic_cast<Behaviour*>(this)) {
			if (nowEffective) {
				scene->QueueBehaviourAdd(behaviour);
			}
			else {
				scene->QueueBehaviourRemove(behaviour);
			}
		}
	}
}

void Component::MarkPendingDestroyInternal() {
	if (!IsPendingDestroy()) {
		MarkPendingDestroy();
	}
}
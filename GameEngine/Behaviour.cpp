#include "Behaviour.h"
#include "GameObject.h"

Behaviour::Behaviour(const std::string& name)
	: Component(name) {
}

void Behaviour::SetEnabled(bool enabled) {
	if (m_enabled == enabled) {
		return;
	}
	m_enabled = enabled;
	OnEnabledStateChanged(enabled);
}

bool Behaviour::IsActiveAndEnabled() const {
	return m_enabled && m_gameObject && m_gameObject->IsActiveInHierarchy() && !IsMarkedForDestruction() && HasOnEnableBeenCalled();
}

bool Behaviour::HasOnEnableBeenCalled() const {
	return true;
}

void Behaviour::OnEnabledStateChanged(bool) {
}
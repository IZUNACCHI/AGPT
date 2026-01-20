#include "Component.h"
#include "GameObject.h"
#include "Scene.h"



	void Component::SetEnabled(bool enabled) {
		if (m_enabled == enabled) return;

		m_enabled = enabled;

		if (m_enabled) {
			OnEnable();
		}
		else {
			OnDisable();
		}
	}

	Scene* Component::GetScene() const {
		return m_gameObject ? m_gameObject->GetScene() : nullptr;
	}

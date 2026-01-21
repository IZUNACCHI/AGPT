#include "Component.h"
#include "GameObject.h"
#include "MonoBehaviour.h"
#include "Transform.h"

Component::Component(const std::string& name)
	: Object(name) {
}

Transform* Component::GetTransform() const {
	return m_gameObject ? m_gameObject->GetTransform() : nullptr;
}

const std::string& Component::GetName() const {
	if (m_gameObject) {
		return m_gameObject->GetName();
	}
	return Object::GetName();
}

void Component::SetName(const std::string& name) {
	if (m_gameObject) {
		m_gameObject->SetName(name);
		return;
	}
	Object::SetName(name);
}

size_t Component::GetComponentIndex() const {
	return m_gameObject ? m_gameObject->GetComponentIndex(this) : 0;
}

void Component::SendMessage(const std::string& methodName) {
	if (m_gameObject) {
		m_gameObject->SendMessage(methodName);
	}
}

void Component::SendMessageUp(const std::string& methodName) {
	if (m_gameObject) {
		m_gameObject->SendMessageUp(methodName);
	}
}

void Component::SendMessageDown(const std::string& methodName) {
	if (m_gameObject) {
		m_gameObject->SendMessageDown(methodName);
	}
}

void Component::DestroyImmediateInternal() {
	if (!m_gameObject) {
		return;
	}

	if (auto behaviour = dynamic_cast<MonoBehaviour*>(this)) {
		behaviour->TriggerDestroy();
	}

	auto owner = m_gameObject;
	m_gameObject = nullptr;
	owner->RemoveComponent(this);
}

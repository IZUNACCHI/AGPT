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

std::shared_ptr<Component> Component::FindComponent(const ComponentPredicate& predicate) const {
	if (!m_gameObject) {
		return nullptr;
	}
	for (const auto& component : m_gameObject->m_components) {
		if (predicate(component)) {
			return component;
		}
	}
	return nullptr;
}

std::shared_ptr<Component> Component::FindComponentInChildren(const ComponentPredicate& predicate) const {
	if (!m_gameObject) {
		return nullptr;
	}
	return FindComponentInChildren(m_gameObject, predicate);
}

std::shared_ptr<Component> Component::FindComponentInParent(const ComponentPredicate& predicate) const {
	if (!m_gameObject) {
		return nullptr;
	}
	auto* parent = m_gameObject->m_transform->GetParent();
	while (parent) {
		auto* parentObject = parent->GetGameObject();
		if (parentObject) {
			for (const auto& component : parentObject->m_components) {
				if (predicate(component)) {
					return component;
				}
			}
		}
		parent = parent->GetParent();
	}
	return nullptr;
}

std::vector<std::shared_ptr<Component>> Component::FindComponents(const ComponentPredicate& predicate) const {
	std::vector<std::shared_ptr<Component>> result;
	if (!m_gameObject) {
		return result;
	}
	for (const auto& component : m_gameObject->m_components) {
		if (predicate(component)) {
			result.push_back(component);
		}
	}
	return result;
}

std::vector<std::shared_ptr<Component>> Component::FindComponentsInChildren(
	const ComponentPredicate& predicate) const {
	std::vector<std::shared_ptr<Component>> result = FindComponents(predicate);
	if (!m_gameObject) {
		return result;
	}
	AppendComponentsInChildren(m_gameObject, predicate, result);
	return result;
}

std::vector<std::shared_ptr<Component>> Component::FindComponentsInParent(
	const ComponentPredicate& predicate) const {
	std::vector<std::shared_ptr<Component>> result;
	if (!m_gameObject) {
		return result;
	}
	auto* parent = m_gameObject->m_transform->GetParent();
	while (parent) {
		auto* parentObject = parent->GetGameObject();
		if (parentObject) {
			for (const auto& component : parentObject->m_components) {
				if (predicate(component)) {
					result.push_back(component);
				}
			}
		}
		parent = parent->GetParent();
	}
	return result;
}

std::shared_ptr<Component> Component::FindComponentInChildren(GameObject* gameObject,
	const ComponentPredicate& predicate) {
	if (!gameObject) {
		return nullptr;
	}
	for (auto* child : gameObject->m_transform->GetChildren()) {
		if (!child) {
			continue;
		}
		auto* childObject = child->GetGameObject();
		if (!childObject) {
			continue;
		}
		for (const auto& component : childObject->m_components) {
			if (predicate(component)) {
				return component;
			}
		}
		if (auto component = FindComponentInChildren(childObject, predicate)) {
			return component;
		}
	}
	return nullptr;
}

void Component::AppendComponentsInChildren(GameObject* gameObject, const ComponentPredicate& predicate,
	std::vector<std::shared_ptr<Component>>& result) {
	if (!gameObject) {
		return;
	}
	for (auto* child : gameObject->m_transform->GetChildren()) {
		if (!child) {
			continue;
		}
		auto* childObject = child->GetGameObject();
		if (!childObject) {
			continue;
		}
		for (const auto& component : childObject->m_components) {
			if (predicate(component)) {
				result.push_back(component);
			}
		}
		AppendComponentsInChildren(childObject, predicate, result);
	}
}

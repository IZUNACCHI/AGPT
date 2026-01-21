#pragma once

#include "Transform.h"

/// Adds a component of type T to this GameObject.
template<typename T, typename... Args>
std::shared_ptr<T> GameObject::AddComponent(Args&&... args) {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	auto component = std::make_shared<T>(std::forward<Args>(args)...);
	component->m_gameObject = this;
	RegisterComponent(component);
	return std::dynamic_pointer_cast<T>(component);
}

/// Returns the first component of type T.
template<typename T>
std::shared_ptr<T> GameObject::GetComponent() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	for (const auto& component : m_components) {
		if (auto casted = std::dynamic_pointer_cast<T>(component)) {
			return casted;
		}
	}
	return nullptr;
}

/// Returns all components of type T.
template<typename T>
std::vector<std::shared_ptr<T> > GameObject::GetComponents() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	std::vector<std::shared_ptr<T> > result;
	for (const auto& component : m_components) {
		if (auto casted = std::dynamic_pointer_cast<T>(component)) {
			result.push_back(casted);
		}
	}
	return result;
}

/// Returns the first component of type T in children.
template<typename T>
std::shared_ptr<T> GameObject::GetComponentInChildren() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	for (auto* child : m_transform->GetChildren()) {
		if (!child) {
			continue;
		}
		if (auto component = child->GetGameObject()->GetComponent<T>()) {
			return component;
		}
		if (auto component = child->GetGameObject()->GetComponentInChildren<T>()) {
			return component;
		}
	}
	return nullptr;
}

/// Returns all components of type T in children.
template<typename T>
std::vector<std::shared_ptr<T> > GameObject::GetComponentsInChildren() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	std::vector<std::shared_ptr<T> > result = GetComponents<T>();
	for (auto* child : m_transform->GetChildren()) {
		if (!child) {
			continue;
		}
		auto childComponents = child->GetGameObject()->GetComponentsInChildren<T>();
		result.insert(result.end(), childComponents.begin(), childComponents.end());
	}
	return result;
}

/// Returns the first component of type T in parents.
template<typename T>
std::shared_ptr<T> GameObject::GetComponentInParent() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	auto* parent = m_transform->GetParent();
	while (parent) {
		if (auto component = parent->GetGameObject()->GetComponent<T>()) {
			return component;
		}
		parent = parent->GetParent();
	}
	return nullptr;
}

/// Returns all components of type T in parents.
template<typename T>
std::vector<std::shared_ptr<T> > GameObject::GetComponentsInParent() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	std::vector<std::shared_ptr<T> > result;
	auto* parent = m_transform->GetParent();
	while (parent) {
		auto components = parent->GetGameObject()->GetComponents<T>();
		result.insert(result.end(), components.begin(), components.end());
		parent = parent->GetParent();
	}
	return result;
}

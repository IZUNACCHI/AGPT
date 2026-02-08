#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "Object.h"

class GameObject;
class Transform;

// Base class for components attached to GameObjects
class Component : public Object {
public:
	// Creates a component with the given name
	explicit Component(const std::string& name = "Component");
	// Destroys the component.
	~Component() override = default;

	// Returns the owning GameObject
	GameObject* GetGameObject() const { return m_gameObject; }
	// Returns the owning GameObject's Transform
	Transform* GetTransform() const;

	// Returns the name of the component (mirrors GameObject name)
	const std::string& GetName() const override;
	// Sets the name on the owning GameObject.
	void SetName(const std::string& name) override;

	// Returns the component's own name (independent of GameObject name)
	const std::string& GetComponentName() const { return Object::GetName(); }
	// Sets the component's own name (independent of GameObject name)
	void SetComponentName(const std::string& name) { Object::SetName(name); }

	// Returns the first component of type T on this GameObject
	template<typename T>
	std::shared_ptr<T> GetComponent();

	// Returns the first component of type T in children
	template<typename T>
	std::shared_ptr<T> GetComponentInChildren();

	// Returns the first component of type T in parents
	template<typename T>
	std::shared_ptr<T> GetComponentInParent();

	// Returns all components of type T on this GameObject
	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponents();

	// Returns all components of type T in children
	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponentsInChildren();

	// Returns all components of type T in parents
	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponentsInParent();

	// Returns the index of this component on its GameObject
	size_t GetComponentIndex() const;

	// Clones the component for instantiation
	virtual std::shared_ptr<Component> Clone() const { return nullptr; }

protected:
	// Allows GameObject to manage component ownership.
	friend class GameObject;
	// Allows Object to manage lifecycle.
	friend class Object;

	// Handles immediate destruction logic for the component.
	void DestroyImmediateInternal() override;

	// Owning GameObject pointer.
	GameObject* m_gameObject = nullptr;

private:
	using ComponentPredicate = std::function<bool(const std::shared_ptr<Component>&)>; // Predicate to match components

	std::shared_ptr<Component> FindComponent(const ComponentPredicate& predicate) const; // Finds first matching component
	std::shared_ptr<Component> FindComponentInChildren(const ComponentPredicate& predicate) const; // Finds first matching component in children
	std::shared_ptr<Component> FindComponentInParent(const ComponentPredicate& predicate) const; // Finds first matching component in parents
	std::vector<std::shared_ptr<Component>> FindComponents(const ComponentPredicate& predicate) const; // Finds all matching components
	std::vector<std::shared_ptr<Component>> FindComponentsInChildren(const ComponentPredicate& predicate) const; // Finds all matching components in children
	std::vector<std::shared_ptr<Component>> FindComponentsInParent(const ComponentPredicate& predicate) const; // Finds all matching components in parents

	static std::shared_ptr<Component> FindComponentInChildren(GameObject* gameObject,
		const ComponentPredicate& predicate); // Static helper to find component in children
	static void AppendComponentsInChildren(GameObject* gameObject,
		const ComponentPredicate& predicate,
		std::vector<std::shared_ptr<Component>>& result); // Static helper to append components in children
};

// Returns the first component of type T on this GameObject.
template<typename T>
std::shared_ptr<T> Component::GetComponent() {
	auto component = FindComponent([](const std::shared_ptr<Component>& entry) {
		return std::dynamic_pointer_cast<T>(entry) != nullptr;
		});
	return std::dynamic_pointer_cast<T>(component);
}

// Returns the first component of type T in children.
template<typename T>
std::shared_ptr<T> Component::GetComponentInChildren() {
	auto component = FindComponentInChildren([](const std::shared_ptr<Component>& entry) {
		return std::dynamic_pointer_cast<T>(entry) != nullptr;
		});
	return std::dynamic_pointer_cast<T>(component);
}

// Returns the first component of type T in parents.
template<typename T>
std::shared_ptr<T> Component::GetComponentInParent() {
	auto component = FindComponentInParent([](const std::shared_ptr<Component>& entry) {
		return std::dynamic_pointer_cast<T>(entry) != nullptr;
		});
	return std::dynamic_pointer_cast<T>(component);
}

// Returns all components of type T on this GameObject.
template<typename T>
std::vector<std::shared_ptr<T>> Component::GetComponents() {
	auto components = FindComponents([](const std::shared_ptr<Component>& entry) {
		return std::dynamic_pointer_cast<T>(entry) != nullptr;
		});
	std::vector<std::shared_ptr<T>> result;
	result.reserve(components.size());
	for (const auto& component : components) {
		if (auto casted = std::dynamic_pointer_cast<T>(component)) {
			result.push_back(casted);
		}
	}
	return result;
}

// Returns all components of type T in children.
template<typename T>
std::vector<std::shared_ptr<T>> Component::GetComponentsInChildren() {
	auto components = FindComponentsInChildren([](const std::shared_ptr<Component>& entry) {
		return std::dynamic_pointer_cast<T>(entry) != nullptr;
		});
	std::vector<std::shared_ptr<T>> result;
	result.reserve(components.size());
	for (const auto& component : components) {
		if (auto casted = std::dynamic_pointer_cast<T>(component)) {
			result.push_back(casted);
		}
	}
	return result;
}

// Returns all components of type T in parents.
template<typename T>
std::vector<std::shared_ptr<T>> Component::GetComponentsInParent() {
	auto components = FindComponentsInParent([](const std::shared_ptr<Component>& entry) {
		return std::dynamic_pointer_cast<T>(entry) != nullptr;
		});
	std::vector<std::shared_ptr<T>> result;
	result.reserve(components.size());
	for (const auto& component : components) {
		if (auto casted = std::dynamic_pointer_cast<T>(component)) {
			result.push_back(casted);
		}
	}
	return result;
}
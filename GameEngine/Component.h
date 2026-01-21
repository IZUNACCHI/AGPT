#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Object.h"
#include "GameObject.h"
#include "Transform.h"


/// Base class for components attached to GameObjects.
class Component : public Object {
public:
	/// Creates a component with the given name.
	explicit Component(const std::string& name = "Component");
	/// Destroys the component.
	~Component() override = default;

	/// Returns the owning GameObject.
	GameObject* GetGameObject() const { return m_gameObject; }
	/// Returns the owning GameObject's Transform.
	Transform* GetTransform() const;

	/// Returns the name of the component (mirrors GameObject name).
	const std::string& GetName() const override;
	/// Sets the name on the owning GameObject.
	void SetName(const std::string& name) override;

	/// Returns the first component of type T on this GameObject.
	template<typename T>
	std::shared_ptr<T> GetComponent();

	/// Returns the first component of type T in children.
	template<typename T>
	std::shared_ptr<T> GetComponentInChildren();

	/// Returns the first component of type T in parents.
	template<typename T>
	std::shared_ptr<T> GetComponentInParent();

	/// Returns all components of type T on this GameObject.
	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponents();

	/// Returns all components of type T in children.
	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponentsInChildren();

	/// Returns all components of type T in parents.
	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponentsInParent();

	/// Returns the index of this component on its GameObject.
	size_t GetComponentIndex() const;

	/// Sends a message to all MonoBehaviours on the GameObject.
	void SendMessage(const std::string& methodName);
	/// Sends a message up the Transform hierarchy.
	void SendMessageUp(const std::string& methodName);
	/// Sends a message down the Transform hierarchy.
	void SendMessageDown(const std::string& methodName);

	/// Clones the component for instantiation.
	virtual std::shared_ptr<Component> Clone() const { return nullptr; }

protected:
	/// Allows GameObject to manage component ownership.
	friend class GameObject;
	/// Allows Object to manage lifecycle.
	friend class Object;

	/// Handles immediate destruction logic for the component.
	void DestroyImmediateInternal() override;

	/// Owning GameObject pointer.
	GameObject* m_gameObject = nullptr;
};

/// Returns the first component of type T on this GameObject.
template<typename T>
std::shared_ptr<T> Component::GetComponent() {
	return m_gameObject ? m_gameObject->GetComponent<T>() : nullptr;
}

/// Returns the first component of type T in children.
template<typename T>
std::shared_ptr<T> Component::GetComponentInChildren() {
	return m_gameObject ? m_gameObject->GetComponentInChildren<T>() : nullptr;
}

/// Returns the first component of type T in parents.
template<typename T>
std::shared_ptr<T> Component::GetComponentInParent() {
	return m_gameObject ? m_gameObject->GetComponentInParent<T>() : nullptr;
}

/// Returns all components of type T on this GameObject.
template<typename T>
std::vector<std::shared_ptr<T>> Component::GetComponents() {
	return m_gameObject ? m_gameObject->GetComponents<T>() : std::vector<std::shared_ptr<T> >();
}

/// Returns all components of type T in children.
template<typename T>
std::vector<std::shared_ptr<T>> Component::GetComponentsInChildren() {
	return m_gameObject ? m_gameObject->GetComponentsInChildren<T>() : std::vector<std::shared_ptr<T> >();
}

/// Returns all components of type T in parents.
template<typename T>
std::vector<std::shared_ptr<T>> Component::GetComponentsInParent() {
	return m_gameObject ? m_gameObject->GetComponentsInParent<T>() : std::vector<std::shared_ptr<T> >();
}
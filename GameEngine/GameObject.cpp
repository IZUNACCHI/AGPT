#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "Object.h"

class Scene;
class Component;
class Behaviour;
class Transform;

// GameObject owns a list of Components and supports parent/child hierarchy.
class GameObject : public Object {
public:
	explicit GameObject(std::string name, Scene* scene);
	~GameObject();

	Scene* GetScene() const { return m_scene; }
	Transform* GetTransform() const { return m_transform.get(); }

	void SetActive(bool active);
	bool IsActiveSelf() const { return m_activeSelf; }
	bool IsActiveInHierarchy() const { return m_activeInHierarchy; }

	void SetParent(GameObject* parent);
	GameObject* GetParent() const { return m_parent; }
	const std::vector<GameObject*>& GetChildren() const { return m_children; }

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args);

	template<typename T>
	T* GetComponent() const;

	template<typename T>
	std::vector<T*> GetComponents() const;

	void Destroy();

private:
	friend class Scene;

	void AddComponentInternal(std::unique_ptr<Component> component);
	void UpdateActiveInHierarchy();
	void SetActiveInHierarchy(bool active);
	void MarkPendingDestroyInternal();

	Scene* m_scene = nullptr;
	GameObject* m_parent = nullptr;
	std::vector<GameObject*> m_children;
	std::vector<std::unique_ptr<Component>> m_components;
	std::unique_ptr<Transform> m_transform;

	bool m_activeSelf = true;
	bool m_activeInHierarchy = true;
};

template<typename T>
T* GameObject::GetComponent() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
	for (const auto& component : m_components) {
		if (auto* casted = dynamic_cast<T*>(component.get())) {
			return casted;
		}
	}
	return nullptr;
}

template<typename T>
std::vector<T*> GameObject::GetComponents() const {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
	std::vector<T*> result;
	for (const auto& component : m_components) {
		if (auto* casted = dynamic_cast<T*>(component.get())) {
			result.push_back(casted);
		}
	}
	return result;
}
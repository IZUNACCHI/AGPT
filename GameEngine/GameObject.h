#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <typeindex>
#include "Transform.h"
#include "Types.hpp"


class Scene;
class Component;
class Behaviour;

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
	GameObject(uint32_t id, const std::string& name, Scene* scene);
	~GameObject();

	// Core methods
	void Start();
	void Update(float deltaTime);
	void FixedUpdate(float fixedDeltaTime);
	void LateUpdate(float deltaTime);

	void SetActive(bool active);
	bool IsActive() const { return m_isActive && m_isActiveInHierarchy; }
	bool IsActiveSelf() const { return m_isActive; }

	// Component management
	template<typename T, typename... Args>
	std::shared_ptr<T> AddComponent(Args&&... args);

	template<typename T>
	std::shared_ptr<T> GetComponent();

	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponents();

	template<typename T>
	std::shared_ptr<T> GetComponentInChildren();

	template<typename T>
	std::vector<std::shared_ptr<T>> GetComponentsInChildren();

	template<typename T>
	std::shared_ptr<T> GetComponentInParent();

	template<typename T>
	void RemoveComponent();

	// Tags and layers
	void SetTag(const std::string& tag) { m_tag = tag; }
	const std::string& GetTag() const { return m_tag; }

	void SetLayer(int layer);
	int GetLayer() const { return m_layer; }

	// Destruction
	void Destroy();
	void DestroyImmediate();
	bool IsMarkedForDestruction() const { return m_markedForDestruction; }

	// Getters
	uint32_t GetID() const { return m_id; }
	const std::string& GetName() const { return m_name; }
	Transform* GetTransform() { return m_transform.get(); }
	Scene* GetScene() { return m_scene; }

	// Component presence checks (for update list optimization)
	bool HasUpdateComponents() const { return m_hasUpdateComponents; }
	bool HasFixedUpdateComponents() const { return m_hasFixedUpdateComponents; }
	bool HasLateUpdateComponents() const { return m_hasLateUpdateComponents; }
	bool HasRenderComponents() const { return m_hasRenderComponents; }

private:
	friend class Scene;
	friend class Component;
	friend class Behaviour;

	void AddComponentInternal(std::shared_ptr<Component> component);
	void RemoveComponentInternal(std::shared_ptr<Component> component);
	void OnComponentAdded(std::shared_ptr<Component> component);
	void OnComponentRemoved(std::shared_ptr<Component> component);
	void UpdateActiveInHierarchy();

	uint32_t m_id;
	std::string m_name;
	std::string m_tag = "Untagged";
	int m_layer = 0;  // Default layer

	bool m_isActive = true;
	bool m_isActiveInHierarchy = true;
	bool m_started = false;
	bool m_markedForDestruction = false;

	std::unique_ptr<Transform> m_transform;
	Scene* m_scene;

	std::vector<std::shared_ptr<Component>> m_components;
	std::unordered_map<std::type_index, std::shared_ptr<Component>> m_componentByType;

	// Flags for optimization
	bool m_hasUpdateComponents = false;
	bool m_hasFixedUpdateComponents = false;
	bool m_hasLateUpdateComponents = false;
	bool m_hasRenderComponents = false;
};

// Template implementations (in header for template methods)
template<typename T, typename... Args>
std::shared_ptr<T> GameObject::AddComponent(Args&&... args) {
	static_assert(std::is_base_of<Component, T>::value,
		"T must derive from Component");

	// Check if component of this type already exists
	auto existing = GetComponent<T>();
	if (existing) {
		return existing;
	}

	auto component = std::make_shared<T>(std::forward<Args>(args)...);
	component->m_gameObject = shared_from_this();

	AddComponentInternal(component);
	OnComponentAdded(component);

	return component;
}

template<typename T>
std::shared_ptr<T> GameObject::GetComponent() {
	static_assert(std::is_base_of<Component, T>::value,
		"T must derive from Component");

	auto it = m_componentByType.find(typeid(T));
	if (it != m_componentByType.end()) {
		return std::static_pointer_cast<T>(it->second);
	}
	return nullptr;
}

template<typename T>
std::vector<std::shared_ptr<T>> GameObject::GetComponents() {
	static_assert(std::is_base_of<Component, T>::value,
		"T must derive from Component");

	std::vector<std::shared_ptr<T>> result;
	for (auto& component : m_components) {
		if (auto casted = std::dynamic_pointer_cast<T>(component)) {
			result.push_back(casted);
		}
	}
	return result;
}

template<typename T>
void GameObject::RemoveComponent() {
	static_assert(std::is_base_of<Component, T>::value,
		"T must derive from Component");

	auto component = GetComponent<T>();
	if (component) {
		RemoveComponentInternal(component);
	}
}

#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include "Behaviour.h"
#include "Component.h"
#include "GameObject.h"

class Behaviour;
class Component;
class GameObject;

// Scene owns all GameObjects and manages lifecycle timing + update passes.
class Scene {
public:
	explicit Scene(std::string name = "");
	~Scene() = default;

	GameObject* CreateGameObject(std::string name = "");

	template<typename T, typename... Args>
	T* AddComponent(GameObject& owner, Args&&... args);

	void Destroy(GameObject* object);

	// Frame flow:
	//   StartFrame() -> FixedStep() (looped externally) -> Update() -> LateUpdate() -> PreRenderFlush().
	void StartFrame();
	void FixedStep(float fixedDt);
	void Update(float dt);
	void LateUpdate(float dt);
	void PreRenderFlush();

	const std::string& GetName() const { return m_name; }

	// Hooks reserved for potential physics integration.
	void RegisterPhysicsComponent(Component*) {}
	void UnregisterPhysicsComponent(Component*) {}

private:
	friend class Component;
	friend class GameObject;

	void QueueBehaviourAdd(Behaviour* behaviour);
	void QueueBehaviourRemove(Behaviour* behaviour);
	void DestroyHierarchy(GameObject* object, std::unordered_set<GameObject*>& visited);
	void RemoveFromUpdateList(const std::unordered_set<Behaviour*>& removeSet);
	void EraseGameObject(GameObject* object);

	std::string m_name;

	std::vector<std::unique_ptr<GameObject>> m_gameObjects;

	// Single update list for Fixed/Update/Late.
	std::vector<Behaviour*> m_updateList;
	std::unordered_set<Behaviour*> m_updateSet;

	// Pending additions/removals for list safety.
	std::vector<Behaviour*> m_pendingAdd;
	std::unordered_set<Behaviour*> m_pendingAddSet;

	std::vector<Behaviour*> m_pendingRemove;
	std::unordered_set<Behaviour*> m_pendingRemoveSet;

	std::vector<GameObject*> m_pendingDestroy;
	std::unordered_set<GameObject*> m_pendingDestroySet;
};


template<typename T, typename... Args>
T* Scene::AddComponent(GameObject& owner, Args&&... args) {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

	auto component = std::make_unique<T>(std::forward<Args>(args)...);
	T* raw = component.get();
	owner.AddComponentInternal(std::move(component));

	// Awake/Init is immediate on instantiation.
	raw->CallAwakeInit();
	// OnEnable is immediate if enabled + active-in-hierarchy.
	raw->OnActiveInHierarchyChanged(owner.IsActiveInHierarchy());
	// OnCreate is immediate in the same frame as instantiation.
	raw->CallOnCreate();

	if (auto* behaviour = dynamic_cast<Behaviour*>(raw)) {
		if (behaviour->IsEligibleForUpdate()) {
			QueueBehaviourAdd(behaviour);
		}
	}

	return raw;
}

template<typename T, typename... Args>
T* GameObject::AddComponent(Args&&... args) {
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
	return m_scene ? m_scene->AddComponent<T>(*this, std::forward<Args>(args)...) : nullptr;
}
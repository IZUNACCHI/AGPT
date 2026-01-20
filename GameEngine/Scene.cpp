#include "Scene.h"

#include <algorithm>

#include "Behaviour.h"
#include "Component.h"
#include "GameObject.h"

Scene::Scene(std::string name)
	: m_name(std::move(name)) {
}

GameObject* Scene::CreateGameObject(std::string name) {
	auto object = std::make_unique<GameObject>(std::move(name), this);
	GameObject* raw = object.get();
	m_gameObjects.emplace_back(std::move(object));
	return raw;
}

void Scene::QueueBehaviourAdd(Behaviour* behaviour) {
	if (!behaviour || m_updateSet.count(behaviour) || m_pendingAddSet.count(behaviour)) {
		return;
	}
	m_pendingAdd.push_back(behaviour);
	m_pendingAddSet.insert(behaviour);
}

void Scene::QueueBehaviourRemove(Behaviour* behaviour) {
	if (!behaviour || m_pendingRemoveSet.count(behaviour)) {
		return;
	}
	m_pendingRemove.push_back(behaviour);
	m_pendingRemoveSet.insert(behaviour);
}

void Scene::Destroy(GameObject* object) {
	if (!object || object->IsPendingDestroy() || m_pendingDestroySet.count(object)) {
		return;
	}

	object->MarkPendingDestroyInternal();
	m_pendingDestroy.push_back(object);
	m_pendingDestroySet.insert(object);

	for (const auto& component : object->m_components) {
		component->MarkPendingDestroyInternal();
		if (auto* behaviour = dynamic_cast<Behaviour*>(component.get())) {
			QueueBehaviourRemove(behaviour);
		}
	}

	for (auto* child : object->m_children) {
		Destroy(child);
	}
}

void Scene::StartFrame() {
	if (m_pendingAdd.empty()) {
		return;
	}

	for (auto* behaviour : m_pendingAdd) {
		if (!behaviour) {
			continue;
		}
		if (!behaviour->IsEligibleForUpdate()) {
			continue;
		}
		if (m_updateSet.insert(behaviour).second) {
			m_updateList.push_back(behaviour);
		}
	}

	m_pendingAdd.clear();
	m_pendingAddSet.clear();
}

void Scene::FixedStep(float fixedDt) {
	for (auto* behaviour : m_updateList) {
		if (behaviour && behaviour->IsEligibleForUpdate()) {
			behaviour->FixedUpdate(fixedDt);
		}
	}
}

void Scene::Update(float dt) {
	for (auto* behaviour : m_updateList) {
		if (behaviour && behaviour->IsEligibleForUpdate()) {
			behaviour->Update(dt);
		}
	}
}

void Scene::LateUpdate(float dt) {
	for (auto* behaviour : m_updateList) {
		if (behaviour && behaviour->IsEligibleForUpdate()) {
			behaviour->LateUpdate(dt);
		}
	}
}

void Scene::RemoveFromUpdateList(const std::unordered_set<Behaviour*>& removeSet) {
	if (removeSet.empty()) {
		return;
	}

	m_updateList.erase(std::remove_if(m_updateList.begin(), m_updateList.end(), [&](Behaviour* behaviour) {
		if (removeSet.count(behaviour)) {
			m_updateSet.erase(behaviour);
			return true;
		}
		return false;
		}), m_updateList.end());
}

void Scene::PreRenderFlush() {
	// Removal pass (disabled/destroyed) happens before destruction.
	RemoveFromUpdateList(m_pendingRemoveSet);
	m_pendingRemove.clear();
	m_pendingRemoveSet.clear();

	if (m_pendingDestroy.empty()) {
		return;
	}

	std::unordered_set<GameObject*> visited;
	for (auto* object : m_pendingDestroy) {
		DestroyHierarchy(object, visited);
	}

	m_pendingDestroy.clear();
	m_pendingDestroySet.clear();
}

void Scene::DestroyHierarchy(GameObject* object, std::unordered_set<GameObject*>& visited) {
	if (!object || visited.count(object)) {
		return;
	}
	visited.insert(object);

	// Children are destroyed before the parent (consistent ordering).
	const auto children = object->m_children;
	for (auto* child : children) {
		DestroyHierarchy(child, visited);
	}

	for (const auto& component : object->m_components) {
		if (component->WasEnabledInHierarchy()) {
			component->OnDisable();
			component->m_effectivelyEnabled = false;
		}

		component->MarkPendingDestroyInternal();

		if (!component->m_destroyCalled) {
			component->m_destroyCalled = true;
			component->OnDestroy();
		}
	}

	// Ensure behaviours are removed from the update list during this flush.
	std::unordered_set<Behaviour*> localRemove;
	for (const auto& component : object->m_components) {
		if (auto* behaviour = dynamic_cast<Behaviour*>(component.get())) {
			localRemove.insert(behaviour);
		}
	}
	RemoveFromUpdateList(localRemove);

	if (object->m_parent) {
		auto& siblings = object->m_parent->m_children;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), object), siblings.end());
	}

	EraseGameObject(object);
}

void Scene::EraseGameObject(GameObject* object) {
	m_gameObjects.erase(std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
		[&](const std::unique_ptr<GameObject>& current) {
			return current.get() == object;
		}),
		m_gameObjects.end());
}
#include "Object.h"
#include "Behaviour.h"
#include "Component.h"
#include "GameObject.h"
#include "MonoBehaviour.h"
#include "Time.hpp"
#include <algorithm>
#include <unordered_map>

namespace {
	uint32_t g_nextInstanceId = 1;
	std::unordered_map<uint32_t, std::weak_ptr<Object>> g_registry;
	struct PendingDestroy {
		std::weak_ptr<Object> object;
		float executeAt = 0.0f;
	};
	std::vector<PendingDestroy> g_destroyQueue;
}

Object::Object(const std::string& name)
	: m_instanceID(g_nextInstanceId++), m_name(name) {
}

Object::~Object() {
	UnregisterObject(m_instanceID);
}

void Object::Destroy(const std::shared_ptr<Object>& obj, float t) {
	if (!obj) {
		return;
	}

	if (obj->m_markedForDestruction) {
		return;
	}

	obj->m_markedForDestruction = true;
	const float executeAt = Time::Now() + (std::max)(0.0f, t);
	QueueDestroy(obj, executeAt);
}

void Object::Destroy(Object* obj, float t) {
	if (!obj) {
		return;
	}
	auto it = g_registry.find(obj->GetInstanceID());
	if (it == g_registry.end()) {
		return;
	}
	if (auto shared = it->second.lock()) {
		Destroy(shared, t);
	}
}

std::shared_ptr<Object> Object::Instantiate(const std::shared_ptr<Object>& obj) {
	if (!obj) {
		return nullptr;
	}
	if (auto gameObject = std::dynamic_pointer_cast<GameObject>(obj)) {
		return gameObject->Clone();
	}
	if (auto component = std::dynamic_pointer_cast<Component>(obj)) {
		auto owner = component->GetGameObject();
		if (owner) {
			return owner->Clone();
		}
	}
	return nullptr;
}

std::vector<std::shared_ptr<Object> > Object::FindObjectsByType(const std::type_index& type, bool includeInactive) {
	std::vector<std::shared_ptr<Object> > result;
	for (const auto& entry : g_registry) {
		if (auto obj = entry.second.lock()) {
			bool typeMatch = false;
			if (type == std::type_index(typeid(Object))) {
				typeMatch = true;
			}
			else if (type == std::type_index(typeid(GameObject))) {
				typeMatch = std::dynamic_pointer_cast<GameObject>(obj) != nullptr;
			}
			else if (type == std::type_index(typeid(Component))) {
				typeMatch = std::dynamic_pointer_cast<Component>(obj) != nullptr;
			}
			else if (type == std::type_index(typeid(Behaviour))) {
				typeMatch = std::dynamic_pointer_cast<Behaviour>(obj) != nullptr;
			}
			else if (type == std::type_index(typeid(MonoBehaviour))) {
				typeMatch = std::dynamic_pointer_cast<MonoBehaviour>(obj) != nullptr;
			}
			else {
				typeMatch = std::type_index(typeid(*obj)) == type;
			}

			if (!typeMatch) {
				continue;
			}

			bool include = true;
			if (!includeInactive) {
				if (auto gameObject = std::dynamic_pointer_cast<GameObject>(obj)) {
					include = gameObject->IsActiveInHierarchy();
				}
				else if (auto component = std::dynamic_pointer_cast<Component>(obj)) {
					auto owner = component->GetGameObject();
					include = owner && owner->IsActiveInHierarchy();
				}
			}

			if (include) {
				result.push_back(obj);
			}
		}
	}
	return result;
}

void Object::ProcessDestroyQueue() {
	if (g_destroyQueue.empty()) {
		return;
	}

	const float now = Time::Now();
	std::vector<PendingDestroy> remaining;
	remaining.reserve(g_destroyQueue.size());

	for (auto& entry : g_destroyQueue) {
		if (entry.executeAt > now) {
			remaining.push_back(entry);
			continue;
		}

		if (auto obj = entry.object.lock()) {
			obj->DestroyImmediateInternal();
			obj->MarkDestroyed();
		}
	}

	g_destroyQueue.swap(remaining);
}

void Object::DestroyImmediateInternal() {
}

void Object::MarkDestroyed() {
	m_destroyed = true;
	UnregisterObject(m_instanceID);
}

void Object::RegisterObject(const std::shared_ptr<Object>& obj) {
	if (!obj) {
		return;
	}
	g_registry[obj->GetInstanceID()] = obj;
}

void Object::UnregisterObject(uint32_t instanceID) {
	g_registry.erase(instanceID);
}

void Object::QueueDestroy(const std::shared_ptr<Object>& obj, float executeAt) {
	PendingDestroy entry;
	entry.object = obj;
	entry.executeAt = executeAt;
	g_destroyQueue.push_back(entry);
}
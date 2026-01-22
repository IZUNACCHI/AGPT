#include "Scene.h"
#include "MonoBehaviour.h"
#include "Time.hpp"
#include <algorithm>

// Global registry of live scenes for cross-scene lookups.
std::vector<Scene*> Scene::s_scenes;

Scene::Scene(const std::string& name)
	: m_name(name) {
	// Register the Scene globally so static lookups can search all active scenes.
	s_scenes.push_back(this);
}

Scene::~Scene() {
	// Ensure objects are destroyed and lifecycle processing stops.
	Unload();
	// Remove this instance from the global registry.
	s_scenes.erase(std::remove(s_scenes.begin(), s_scenes.end(), this), s_scenes.end());
}

void Scene::Start() {
	// Only start once; subsequent calls are ignored.
	if (m_isActive) {
		return;
	}
	m_isActive = true;
	// Give derived scenes a hook to initialize runtime state.
	OnStart();
	// Ensure each object recalculates its active state before lifecycle runs.
	for (const auto& obj : m_allGameObjects) {
		obj->UpdateActiveInHierarchy();
		// Queue lifecycle events for all behaviours in the scene.
		for (const auto& component : obj->GetComponents<MonoBehaviour>()) {
			QueueLifecycle(component.get());
		}
	}
	// Process Awake/Enable/Start for any queued behaviours.
	ProcessLifecycleQueue();
}

void Scene::Update(float deltaTime) {
	// Skip updates if the scene has not started or has been unloaded.
	if (!m_isActive) {
		return;
	}

	// Run lifecycle before update so newly queued behaviours are ready.
	ProcessLifecycleQueue();
	// Call derived scene update logic.
	OnUpdate(deltaTime);

	const float now = Time::Now();
	for (const auto& obj : m_allGameObjects) {
		// Skip inactive hierarchy branches.
		if (!obj->IsActiveInHierarchy()) {
			continue;
		}
		for (const auto& behaviour : obj->GetComponents<MonoBehaviour>()) {
			if (behaviour->IsActiveAndEnabled()) {
				// Update invoke timers before Update().
				behaviour->TickInvokes(now);
				behaviour->Update(deltaTime);
			}
		}
	}

	// Run lifecycle again in case Update() queued new behaviours or state transitions.
	ProcessLifecycleQueue();
}

void Scene::FixedUpdate(float fixedDeltaTime) {
	// Skip fixed update if the scene is inactive.
	if (!m_isActive) {
		return;
	}
	// Call derived scene fixed-step logic.
	OnFixedUpdate(fixedDeltaTime);
	for (const auto& obj : m_allGameObjects) {
		if (!obj->IsActiveInHierarchy()) {
			continue;
		}
		for (const auto& behaviour : obj->GetComponents<MonoBehaviour>()) {
			if (behaviour->IsActiveAndEnabled()) {
				behaviour->FixedUpdate(fixedDeltaTime);
			}
		}
	}
}

void Scene::LateUpdate(float deltaTime) {
	// Skip late update if the scene is inactive.
	if (!m_isActive) {
		return;
	}
	// Call derived scene late update logic.
	OnLateUpdate(deltaTime);
	for (const auto& obj : m_allGameObjects) {
		if (!obj->IsActiveInHierarchy()) {
			continue;
		}
		for (const auto& behaviour : obj->GetComponents<MonoBehaviour>()) {
			if (behaviour->IsActiveAndEnabled()) {
				behaviour->LateUpdate(deltaTime);
			}
		}
	}
}

void Scene::Render() {
	// Rendering is only executed for active scenes.
	if (!m_isActive) {
		return;
	}
	OnRender();
}

void Scene::Unload() {
	// Prevent double-unload; this can be called from the destructor.
	if (m_markedForUnload) {
		return;
	}
	m_markedForUnload = true;
	m_isActive = false;
	// Allow derived scenes to clean up resources.
	OnDestroy();

	// Destroy all objects registered with this scene.
	for (const auto& obj : m_allGameObjects) {
		Object::Destroy(obj);
	}
}

template<typename T, typename... Args>
std::shared_ptr<T> Scene::CreateGameObject(const std::string& name, Args&&... args) {
	static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");
	auto obj = std::make_shared<T>(name, std::forward<Args>(args)...);
	Object::RegisterObject(obj);
	AdoptGameObject(obj);
	return obj;
}

std::shared_ptr<GameObject> Scene::FindGameObject(const std::string& nameOrPath) {
	// Handle empty input early.
	if (nameOrPath.empty()) {
		return nullptr;
	}

	// Search through every active scene.
	for (const auto* scene : s_scenes) {
		if (!scene) {
			continue;
		}
		// If no path separators are present, search by name only.
		if (nameOrPath.find('/') == std::string::npos) {
			for (const auto& obj : scene->m_allGameObjects) {
				if (obj && obj->GetName() == nameOrPath) {
					return obj;
				}
			}
			continue;
		}

		// Split a hierarchical path like Root/Child/GrandChild into tokens.
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t slash = 0;
		while ((slash = nameOrPath.find('/', start)) != std::string::npos) {
			tokens.push_back(nameOrPath.substr(start, slash - start));
			start = slash + 1;
		}
		tokens.push_back(nameOrPath.substr(start));

		// Walk each root object that matches the first token and traverse children.
		for (const auto& root : scene->m_rootGameObjects) {
			if (!root || root->GetName() != tokens.front()) {
				continue;
			}

			GameObject* current = root.get();
			for (size_t i = 1; i < tokens.size() && current; ++i) {
				auto* transform = current->GetTransform();
				GameObject* next = nullptr;
				for (auto* child : transform->GetChildren()) {
					if (child && child->GetGameObject()->GetName() == tokens[i]) {
						next = child->GetGameObject();
						break;
					}
				}
				current = next;
			}

			// Resolve the found object back to the Scene-owned shared_ptr.
			if (current) {
				auto found = scene->m_gameObjectById.find(current->GetInstanceID());
				if (found != scene->m_gameObjectById.end()) {
					return found->second;
				}
			}
		}
	}
	return nullptr;
}

void Scene::QueueLifecycle(MonoBehaviour* behaviour) {
	// Prevent null entries and duplicates in the queue.
	if (!behaviour) {
		return;
	}
	if (std::find(m_pendingLifecycle.begin(), m_pendingLifecycle.end(), behaviour) == m_pendingLifecycle.end()) {
		m_pendingLifecycle.push_back(behaviour);
	}
}

void Scene::ProcessLifecycleQueue() {
	// Nothing to do if no behaviours were queued.
	if (m_pendingLifecycle.empty()) {
		return;
	}

	// Drain the queue so new lifecycle work can be queued during processing.
	std::vector<MonoBehaviour*> pending;
	pending.swap(m_pendingLifecycle);

	// First pass handles Awake and Enable, and collects Start candidates.
	std::vector<MonoBehaviour*> startCandidates;
	for (auto* behaviour : pending) {
		if (!behaviour || behaviour->IsDestroyed()) {
			continue;
		}
		auto* owner = behaviour->GetGameObject();
		if (!owner || !owner->IsActiveInHierarchy()) {
			continue;
		}

		if (!behaviour->DidAwake()) {
			behaviour->TriggerAwake();
		}

		if (behaviour->IsEnabled()) {
			behaviour->TriggerEnable();
			if (!behaviour->DidStart()) {
				startCandidates.push_back(behaviour);
			}
		}
	}

	// Second pass runs Start after Awake/Enable so ordering is consistent.
	for (auto* behaviour : startCandidates) {
		if (!behaviour || behaviour->IsDestroyed()) {
			continue;
		}
		auto* owner = behaviour->GetGameObject();
		if (!owner || !owner->IsActiveInHierarchy() || !behaviour->IsEnabled()) {
			continue;
		}
		behaviour->TriggerStart();
	}
}

void Scene::AdoptGameObject(const std::shared_ptr<GameObject>& obj) {
	// Store the GameObject in scene-owned collections for lifecycle and lookup.
	if (!obj) {
		return;
	}
	obj->SetScene(this);
	m_allGameObjects.push_back(obj);
	m_gameObjectById[obj->GetInstanceID()] = obj;

	// Track root objects separately for hierarchy traversal and lookups.
	if (!obj->GetTransform()->GetParent()) {
		m_rootGameObjects.push_back(obj);
	}
}

void Scene::RemoveGameObject(GameObject* obj) {
	// Remove references when a GameObject is destroyed.
	if (!obj) {
		return;
	}

	auto it = std::remove_if(m_allGameObjects.begin(), m_allGameObjects.end(),
		[obj](const std::shared_ptr<GameObject>& entry) { return entry.get() == obj; });
	m_allGameObjects.erase(it, m_allGameObjects.end());

	auto rootIt = std::remove_if(m_rootGameObjects.begin(), m_rootGameObjects.end(),
		[obj](const std::shared_ptr<GameObject>& entry) { return entry.get() == obj; });
	m_rootGameObjects.erase(rootIt, m_rootGameObjects.end());

	m_gameObjectById.erase(obj->GetInstanceID());
}

void Scene::UpdateRootGameObject(GameObject* obj) {
	// Update the root list when an object's parent changes.
	if (!obj) {
		return;
	}

	bool isRoot = obj->GetTransform() && obj->GetTransform()->GetParent() == nullptr;
	auto hasRoot = std::find_if(m_rootGameObjects.begin(), m_rootGameObjects.end(),
		[obj](const std::shared_ptr<GameObject>& entry) { return entry.get() == obj; });

	if (isRoot) {
		// Add the object if it is now a root and is not already tracked.
		if (hasRoot == m_rootGameObjects.end()) {
			auto found = m_gameObjectById.find(obj->GetInstanceID());
			if (found != m_gameObjectById.end()) {
				m_rootGameObjects.push_back(found->second);
			}
		}
	}
	else if (hasRoot != m_rootGameObjects.end()) {
		// Remove the object if it is no longer a root.
		m_rootGameObjects.erase(hasRoot);
	}
}

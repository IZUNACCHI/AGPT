#include "Scene.h"
#include "MonoBehaviour.h"
#include "Time.hpp"
#include <algorithm>

std::vector<Scene*> Scene::s_scenes;

Scene::Scene(const std::string& name)
	: m_name(name) {
	s_scenes.push_back(this);
}

Scene::~Scene() {
	Unload();
	s_scenes.erase(std::remove(s_scenes.begin(), s_scenes.end(), this), s_scenes.end());
}

std::shared_ptr<GameObject> Scene::CreateGameObject(const std::string& name) {
	auto obj = std::make_shared<GameObject>(name);
	Object::RegisterObject(obj);
	AdoptGameObject(obj);
	return obj;
}

std::shared_ptr<GameObject> Scene::CreateGameObject(const std::string& name, Transform* parent) {
	auto obj = CreateGameObject(name);
	if (parent) {
		obj->GetTransform()->SetParent(parent);
	}
	return obj;
}

void Scene::Start() {
	if (m_isActive) {
		return;
	}
	m_isActive = true;
	OnStart();
	for (const auto& obj : m_allGameObjects) {
		obj->UpdateActiveInHierarchy();
		for (const auto& component : obj->GetComponents<MonoBehaviour>()) {
			QueueLifecycle(component.get());
		}
	}
	ProcessLifecycleQueue();
}

void Scene::Update(float deltaTime) {
	if (!m_isActive) {
		return;
	}

	ProcessLifecycleQueue();
	OnUpdate(deltaTime);

	const float now = Time::Now();
	for (const auto& obj : m_allGameObjects) {
		if (!obj->IsActiveInHierarchy()) {
			continue;
		}
		for (const auto& behaviour : obj->GetComponents<MonoBehaviour>()) {
			if (behaviour->IsActiveAndEnabled()) {
				behaviour->TickInvokes(now);
				behaviour->Update(deltaTime);
			}
		}
	}

	ProcessLifecycleQueue();
}

void Scene::FixedUpdate(float fixedDeltaTime) {
	if (!m_isActive) {
		return;
	}
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
	if (!m_isActive) {
		return;
	}
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
	if (!m_isActive) {
		return;
	}
	OnRender();
}

void Scene::Unload() {
	if (m_markedForUnload) {
		return;
	}
	m_markedForUnload = true;
	m_isActive = false;
	OnDestroy();

	for (const auto& obj : m_allGameObjects) {
		Object::Destroy(obj);
	}
}

std::shared_ptr<GameObject> Scene::FindGameObject(const std::string& nameOrPath) {
	if (nameOrPath.empty()) {
		return nullptr;
	}

	for (const auto* scene : s_scenes) {
		if (!scene) {
			continue;
		}
		if (nameOrPath.find('/') == std::string::npos) {
			for (const auto& obj : scene->m_allGameObjects) {
				if (obj && obj->GetName() == nameOrPath) {
					return obj;
				}
			}
			continue;
		}

		std::vector<std::string> tokens;
		size_t start = 0;
		size_t slash = 0;
		while ((slash = nameOrPath.find('/', start)) != std::string::npos) {
			tokens.push_back(nameOrPath.substr(start, slash - start));
			start = slash + 1;
		}
		tokens.push_back(nameOrPath.substr(start));

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
	if (!behaviour) {
		return;
	}
	if (std::find(m_pendingLifecycle.begin(), m_pendingLifecycle.end(), behaviour) == m_pendingLifecycle.end()) {
		m_pendingLifecycle.push_back(behaviour);
	}
}

void Scene::ProcessLifecycleQueue() {
	if (m_pendingLifecycle.empty()) {
		return;
	}

	std::vector<MonoBehaviour*> pending;
	pending.swap(m_pendingLifecycle);

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
	if (!obj) {
		return;
	}
	obj->SetScene(this);
	m_allGameObjects.push_back(obj);
	m_gameObjectById[obj->GetInstanceID()] = obj;

	if (!obj->GetTransform()->GetParent()) {
		m_rootGameObjects.push_back(obj);
	}
}

void Scene::RemoveGameObject(GameObject* obj) {
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
	if (!obj) {
		return;
	}

	bool isRoot = obj->GetTransform() && obj->GetTransform()->GetParent() == nullptr;
	auto hasRoot = std::find_if(m_rootGameObjects.begin(), m_rootGameObjects.end(),
		[obj](const std::shared_ptr<GameObject>& entry) { return entry.get() == obj; });

	if (isRoot) {
		if (hasRoot == m_rootGameObjects.end()) {
			auto found = m_gameObjectById.find(obj->GetInstanceID());
			if (found != m_gameObjectById.end()) {
				m_rootGameObjects.push_back(found->second);
			}
		}
	}
	else if (hasRoot != m_rootGameObjects.end()) {
		m_rootGameObjects.erase(hasRoot);
	}
}

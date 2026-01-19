#include "Scene.h"
#include "GameObject.h"
#include "Component.h"
#include "Behaviour.h"
#include "RenderableComponent.h"
#include "Logger.h"
#include <algorithm>


	uint32_t Scene::s_nextGameObjectID = 1;

	Scene::Scene(const std::string& name) : m_name(name) {
		// Initialize default layer names (similar to Unity)
		m_layerNames = {
			"Default", "TransparentFX", "Ignore Raycast", "", "",
			"Water", "UI", "", "", "",
			"", "", "", "", "", "",
			"", "", "", "", "",
			"", "", "", "", "",
			"", "", "", "", "", ""
		};

		// Initialize render orders (lower = rendered first)
		for (int i = 0; i < MAX_LAYERS; ++i) {
			m_layerRenderOrders[i] = i;
		}

		// Build layer name to index map
		for (int i = 0; i < MAX_LAYERS; ++i) {
			if (!m_layerNames[i].empty()) {
				m_layerNameToIndex[m_layerNames[i]] = i;
			}
		}
	}

	Scene::~Scene() {
		Unload();
	}

	void Scene::Start() {
		if (m_isActive) return;

		m_isActive = true;
		OnStart();

		// Call Start on all game objects
		for (auto& obj : m_allGameObjects) {
			if (obj->IsActive()) {
				obj->Start();
			}
		}
	}

	void Scene::Update(float deltaTime) {
		if (!m_isActive) return;

		OnUpdate(deltaTime);

		// Update all active game objects
		for (auto& obj : m_activeGameObjects) {
			if (obj->IsActive()) {
				obj->Update(deltaTime);
			}
		}
	}

	void Scene::FixedUpdate(float fixedDeltaTime) {
		if (!m_isActive) return;

		OnFixedUpdate(fixedDeltaTime);

		// FixedUpdate all relevant game objects
		for (auto& obj : m_fixedUpdateObjects) {
			if (obj->IsActive()) {
				obj->FixedUpdate(fixedDeltaTime);
			}
		}
	}

	void Scene::LateUpdate(float deltaTime) {
		if (!m_isActive) return;

		OnLateUpdate(deltaTime);

		// LateUpdate all relevant game objects
		for (auto& obj : m_lateUpdateObjects) {
			if (obj->IsActive()) {
				obj->LateUpdate(deltaTime);
			}
		}
	}

	void Scene::Render() {
		if (!m_isActive) return;

		OnRender();

		// Sort render components by layer order
		std::sort(m_renderComponents.begin(), m_renderComponents.end(),
			[this](const std::shared_ptr<Component>& a, const std::shared_ptr<Component>& b) {
				int layerA = a->GetGameObject()->GetLayer();
				int layerB = b->GetGameObject()->GetLayer();

				// First sort by layer order
				if (m_layerRenderOrders[layerA] != m_layerRenderOrders[layerB]) {
					return m_layerRenderOrders[layerA] < m_layerRenderOrders[layerB];
				}

				// Then sort by render order within same layer
				auto renderableA = std::dynamic_pointer_cast<RenderableComponent>(a);
				auto renderableB = std::dynamic_pointer_cast<RenderableComponent>(b);
				if (renderableA && renderableB) {
					return renderableA->GetRenderOrder() < renderableB->GetRenderOrder();
				}

				return false;
			});

		// Render all components
		for (auto& component : m_renderComponents) {
			if (auto renderable = std::dynamic_pointer_cast<RenderableComponent>(component)) {
				if (renderable->IsVisible() && component->GetGameObject()->IsActive()) {
					component->Draw();
				}
			}
		}
	}

	void Scene::Unload() {
		if (m_markedForUnload) return;

		m_markedForUnload = true;
		m_isActive = false;

		OnDestroy();

		// Mark all game objects for destruction
		for (auto& obj : m_allGameObjects) {
			obj->Destroy();
		}

		ProcessDestructionQueue();
	}

	void Scene::Clear() {
		// Destroy all game objects
		for (auto& obj : m_allGameObjects) {
			obj->Destroy();
		}

		// Process destruction queue immediately
		ProcessDestructionQueue();
	}

	std::shared_ptr<GameObject> Scene::CreateGameObject(const std::string& name) {
		auto obj = std::make_shared<GameObject>(s_nextGameObjectID++, name, this);
		m_rootGameObjects.push_back(obj);
		m_allGameObjects.push_back(obj);
		m_gameObjectByID[obj->GetID()] = obj;

		return obj;
	}

	std::shared_ptr<GameObject> Scene::CreateGameObject(const std::string& name, Transform* parent) {
		auto obj = CreateGameObject(name);
		if (parent) {
			obj->GetTransform()->SetParent(parent);
		}
		return obj;
	}

	void Scene::DestroyGameObject(std::shared_ptr<GameObject> obj) {
		if (!obj) return;
		m_destructionQueue.push_back(obj);
	}

	void Scene::DestroyGameObjectImmediate(std::shared_ptr<GameObject> obj) {
		if (!obj) return;

		// Remove from update lists first
		RemoveFromUpdateList(obj);

		// Remove all components from render list
		auto components = obj->GetComponents<Component>();
		for (auto& component : components) {
			RemoveFromRenderList(component);
		}

		// Remove from all lists
		auto it = std::find(m_allGameObjects.begin(), m_allGameObjects.end(), obj);
		if (it != m_allGameObjects.end()) {
			m_allGameObjects.erase(it);
		}

		// Remove from root objects if it's a root
		auto rootIt = std::find(m_rootGameObjects.begin(), m_rootGameObjects.end(), obj);
		if (rootIt != m_rootGameObjects.end()) {
			m_rootGameObjects.erase(rootIt);
		}

		// Remove from ID map
		m_gameObjectByID.erase(obj->GetID());

		// Destroy the object
		obj->DestroyImmediate();
	}

	void Scene::ProcessDestructionQueue() {
		if (m_destructionQueue.empty()) return;

		for (auto& obj : m_destructionQueue) {
			DestroyGameObjectImmediate(obj);
		}
		m_destructionQueue.clear();
	}

	void Scene::AddToUpdateList(std::shared_ptr<GameObject> obj) {
		// Check if object needs to be in update lists
		if (obj->HasUpdateComponents() &&
			std::find(m_activeGameObjects.begin(), m_activeGameObjects.end(), obj) == m_activeGameObjects.end()) {
			m_activeGameObjects.push_back(obj);
		}
		if (obj->HasFixedUpdateComponents() &&
			std::find(m_fixedUpdateObjects.begin(), m_fixedUpdateObjects.end(), obj) == m_fixedUpdateObjects.end()) {
			m_fixedUpdateObjects.push_back(obj);
		}
		if (obj->HasLateUpdateComponents() &&
			std::find(m_lateUpdateObjects.begin(), m_lateUpdateObjects.end(), obj) == m_lateUpdateObjects.end()) {
			m_lateUpdateObjects.push_back(obj);
		}
	}

	void Scene::AddToRenderList(std::shared_ptr<Component> component) {
		if (std::find(m_renderComponents.begin(), m_renderComponents.end(), component) == m_renderComponents.end()) {
			m_renderComponents.push_back(component);
		}
	}

	void Scene::RemoveFromUpdateList(std::shared_ptr<GameObject> obj) {
		auto removeFromVector = [&obj](std::vector<std::shared_ptr<GameObject>>& vec) {
			auto it = std::find(vec.begin(), vec.end(), obj);
			if (it != vec.end()) {
				vec.erase(it);
			}
			};

		removeFromVector(m_activeGameObjects);
		removeFromVector(m_fixedUpdateObjects);
		removeFromVector(m_lateUpdateObjects);
	}

	void Scene::RemoveFromRenderList(std::shared_ptr<Component> component) {
		auto it = std::find(m_renderComponents.begin(), m_renderComponents.end(), component);
		if (it != m_renderComponents.end()) {
			m_renderComponents.erase(it);
		}
	}

	std::shared_ptr<GameObject> Scene::FindGameObjectByName(const std::string& name) {
		for (auto& obj : m_allGameObjects) {
			if (obj->GetName() == name) {
				return obj;
			}
		}
		return nullptr;
	}

	std::vector<std::shared_ptr<GameObject>> Scene::FindGameObjectsByTag(const std::string& tag) {
		std::vector<std::shared_ptr<GameObject>> result;
		for (auto& obj : m_allGameObjects) {
			if (obj->GetTag() == tag) {
				result.push_back(obj);
			}
		}
		return result;
	}

	std::shared_ptr<GameObject> Scene::FindGameObjectByID(uint32_t id) {
		auto it = m_gameObjectByID.find(id);
		if (it != m_gameObjectByID.end()) {
			return it->second;
		}
		return nullptr;
	}

	void Scene::SetLayerName(int layerIndex, const std::string& layerName) {
		if (layerIndex < 0 || layerIndex >= MAX_LAYERS) {
			return;
		}

		// Remove old name from map
		if (!m_layerNames[layerIndex].empty()) {
			m_layerNameToIndex.erase(m_layerNames[layerIndex]);
		}

		m_layerNames[layerIndex] = layerName;

		if (!layerName.empty()) {
			m_layerNameToIndex[layerName] = layerIndex;
		}
	}

	const std::string& Scene::GetLayerName(int layerIndex) const {
		static const std::string emptyString;

		if (layerIndex < 0 || layerIndex >= MAX_LAYERS) {
			return emptyString;
		}

		return m_layerNames[layerIndex];
	}

	int Scene::GetLayerIndex(const std::string& layerName) const {
		auto it = m_layerNameToIndex.find(layerName);
		if (it != m_layerNameToIndex.end()) {
			return it->second;
		}
		return -1; // Layer not found
	}

	void Scene::SetRenderOrderForLayer(int layerIndex, int order) {
		if (layerIndex < 0 || layerIndex >= MAX_LAYERS) {
			return;
		}

		m_layerRenderOrders[layerIndex] = order;
	}

	int Scene::GetRenderOrderForLayer(int layerIndex) const {
		if (layerIndex < 0 || layerIndex >= MAX_LAYERS) {
			return 0;
		}

		return m_layerRenderOrders[layerIndex];
	}

#include "GameObject.h"
#include "Scene.h"
#include "Component.h"
#include "Behaviour.h"
#include "RenderableComponent.h"
#include "Logger.h"
#include <algorithm>
#include <typeinfo>

namespace Engine {

	GameObject::GameObject(uint32_t id, const std::string& name, Scene* scene)
		: m_id(id), m_name(name), m_scene(scene) {
		m_transform = std::make_unique<Transform>(this);
	}

	GameObject::~GameObject() {
		// Destroy all components
		for (auto& component : m_components) {
			component->OnDestroy();
		}
		m_components.clear();
		m_componentByType.clear();
	}

	void GameObject::Start() {
		if (m_started) return;
		m_started = true;

		// Call Start on all behaviour components
		for (auto& component : m_components) {
			if (auto behaviour = std::dynamic_pointer_cast<Behaviour>(component)) {
				behaviour->Start();
			}
		}
	}

	void GameObject::Update(float deltaTime) {
		for (auto& component : m_components) {
			if (component->IsEnabled()) {
				component->Update(deltaTime);
			}
		}
	}

	void GameObject::FixedUpdate(float fixedDeltaTime) {
		for (auto& component : m_components) {
			if (component->IsEnabled()) {
				component->FixedUpdate(fixedDeltaTime);
			}
		}
	}

	void GameObject::LateUpdate(float deltaTime) {
		for (auto& component : m_components) {
			if (component->IsEnabled()) {
				component->LateUpdate(deltaTime);
			}
		}
	}

	void GameObject::SetActive(bool active) {
		if (m_isActive == active) return;

		m_isActive = active;

		if (active) {
			// Call OnEnable on all components
			for (auto& component : m_components) {
				component->OnEnable();
			}

			// If this is the first activation, call Start
			if (!m_started && m_isActiveInHierarchy) {
				Start();
			}
		}
		else {
			// Call OnDisable on all components
			for (auto& component : m_components) {
				component->OnDisable();
			}
		}

		// Update active in hierarchy for children
		UpdateActiveInHierarchy();
	}

	void GameObject::SetLayer(int layer) {
		if (layer < 0 || layer >= Scene::MAX_LAYERS) {
			return;
		}
		m_layer = layer;
	}

	void GameObject::Destroy() {
		if (m_markedForDestruction) return;

		m_markedForDestruction = true;
		SetActive(false);

		// Add to scene's destruction queue
		if (m_scene) {
			m_scene->DestroyGameObject(shared_from_this());
		}
	}

	void GameObject::DestroyImmediate() {
		// Mark all children for destruction
		auto children = m_transform->GetChildren();
		for (auto child : children) {
			child->GetGameObject()->DestroyImmediate();
		}

		// Call OnDisable and OnDestroy on all components
		for (auto& component : m_components) {
			if (component->IsEnabled()) {
				component->OnDisable();
			}
			component->OnDestroy();
		}

		// Clear components
		m_components.clear();
		m_componentByType.clear();
	}

	void GameObject::AddComponentInternal(std::shared_ptr<Component> component) {
		m_components.push_back(component);
		m_componentByType[typeid(*component)] = component;
	}

	void GameObject::RemoveComponentInternal(std::shared_ptr<Component> component) {
		// Remove from type map
		m_componentByType.erase(typeid(*component));

		// Remove from vector
		auto it = std::find(m_components.begin(), m_components.end(), component);
		if (it != m_components.end()) {
			m_components.erase(it);
		}
	}

	void GameObject::OnComponentAdded(std::shared_ptr<Component> component) {
		// Update flags for update lists
		if (dynamic_cast<Behaviour*>(component.get())) {
			m_hasUpdateComponents = true;
			m_hasFixedUpdateComponents = true;
			m_hasLateUpdateComponents = true;
		}

		if (dynamic_cast<RenderableComponent*>(component.get())) {
			m_hasRenderComponents = true;
		}

		// Add to scene's update lists if needed
		if (m_scene) {
			m_scene->AddToUpdateList(shared_from_this());

			if (m_hasRenderComponents) {
				m_scene->AddToRenderList(component);
			}
		}

		// Call OnCreate
		component->OnCreate();

		// If game object is active, call OnEnable
		if (IsActive()) {
			component->OnEnable();
		}

		// If game object has started, call Start on behaviour components
		if (m_started && IsActive()) {
			if (auto behaviour = std::dynamic_pointer_cast<Behaviour>(component)) {
				behaviour->Start();
			}
		}
	}

	void GameObject::OnComponentRemoved(std::shared_ptr<Component> component) {
		// Call OnDisable and OnDestroy
		if (component->IsEnabled()) {
			component->OnDisable();
		}
		component->OnDestroy();

		// Update flags (simplified - would need to check all remaining components)
		if (dynamic_cast<Behaviour*>(component.get())) {
			// Need to check if any other behaviour components exist
			bool hasBehaviour = false;
			for (auto& comp : m_components) {
				if (dynamic_cast<Behaviour*>(comp.get())) {
					hasBehaviour = true;
					break;
				}
			}
			m_hasUpdateComponents = hasBehaviour;
			m_hasFixedUpdateComponents = hasBehaviour;
			m_hasLateUpdateComponents = hasBehaviour;
		}

		if (dynamic_cast<RenderableComponent*>(component.get())) {
			bool hasRenderable = false;
			for (auto& comp : m_components) {
				if (dynamic_cast<RenderableComponent*>(comp.get())) {
					hasRenderable = true;
					break;
				}
			}
			m_hasRenderComponents = hasRenderable;
		}
	}

	void GameObject::UpdateActiveInHierarchy() {
		bool parentActive = true;
		if (auto parent = m_transform->GetParent()) {
			parentActive = parent->GetGameObject()->IsActive();
		}

		bool wasActive = m_isActiveInHierarchy;
		m_isActiveInHierarchy = m_isActive && parentActive;

		if (wasActive != m_isActiveInHierarchy) {
			if (m_isActiveInHierarchy) {
				// Became active
				for (auto& component : m_components) {
					if (component->IsEnabled()) {
						component->OnEnable();
					}
				}

				// Start if not started yet
				if (!m_started) {
					Start();
				}
			}
			else {
				// Became inactive
				for (auto& component : m_components) {
					if (component->IsEnabled()) {
						component->OnDisable();
					}
				}
			}
		}

		// Update children
		for (auto child : m_transform->GetChildren()) {
			child->GetGameObject()->UpdateActiveInHierarchy();
		}
	}
}
#include "GameObject.h"

#include <algorithm>

#include "Component.h"
#include "EngineException.hpp"
#include "MonoBehaviour.h"
#include "Scene.h"
#include "Transform.h"

// Physics components used for rules/enforcement
#include "Rigidbody2D.h"
#include "Collider2D.h"

GameObject::GameObject(const std::string& name)
	: Object(name) {
	m_transform = std::make_shared<Transform>(this);
	RegisterComponent(m_transform);
}

GameObject::~GameObject() {
	// Ensure behaviours get destruction callbacks.
	for (const auto& component : m_components) {
		if (auto behaviour = std::dynamic_pointer_cast<MonoBehaviour>(component)) {
			behaviour->TriggerDestroy();
		}
	}
	m_components.clear();
}

void GameObject::SetActive(bool value) {
	if (m_activeSelf == value) {
		return;
	}
	m_activeSelf = value;
	UpdateActiveInHierarchy();
}

size_t GameObject::GetComponentIndex(const Component* component) const {
	for (size_t i = 0; i < m_components.size(); ++i) {
		if (m_components[i].get() == component) {
			return i;
		}
	}
	return 0;
}

std::shared_ptr<Component> GameObject::GetComponentByName(const std::string& componentName) const {
	for (const auto& component : m_components) {
		if (!component) continue;
		if (component->GetComponentName() == componentName) {
			return component;
		}
	}
	return nullptr;
}

std::shared_ptr<GameObject> GameObject::Clone() const {
	auto clone = std::make_shared<GameObject>(GetName());
	Object::RegisterObject(clone);

	if (m_scene) {
		m_scene->AdoptGameObject(clone);
	}

	clone->m_activeSelf = m_activeSelf;
	clone->m_layer = m_layer;

	if (m_transform) {
		clone->m_transform->SetPosition(m_transform->GetPosition());
		clone->m_transform->SetRotation(m_transform->GetRotation());
		clone->m_transform->SetScale(m_transform->GetScale());
	}

	// Clone Rigidbody2D first so colliders validate correctly.
	for (const auto& component : m_components) {
		if (!component) continue;
		if (component.get() == m_transform.get()) continue;

		if (!std::dynamic_pointer_cast<Rigidbody2D>(component)) {
			continue;
		}

		auto cloneComponent = component->Clone();
		if (!cloneComponent) continue;

		cloneComponent->m_gameObject = clone.get();
		clone->RegisterComponent(cloneComponent);
	}

	// Clone remaining components.
	for (const auto& component : m_components) {
		if (!component) continue;
		if (component.get() == m_transform.get()) continue;
		if (std::dynamic_pointer_cast<Rigidbody2D>(component)) continue;

		auto cloneComponent = component->Clone();
		if (!cloneComponent) continue;

		cloneComponent->m_gameObject = clone.get();
		clone->RegisterComponent(cloneComponent);
	}

	clone->UpdateActiveInHierarchy();
	return clone;
}

std::shared_ptr<GameObject> GameObject::Find(const std::string& nameOrPath) {
	return Scene::FindGameObject(nameOrPath);
}

Scene* GameObject::GetScene(int instanceID) {
	auto matches = Object::FindObjectsByType<GameObject>(true);
	for (const auto& gameObject : matches) {
		if (gameObject && static_cast<int>(gameObject->GetInstanceID()) == instanceID) {
			return gameObject->GetScene();
		}
	}
	return nullptr;
}

void GameObject::SetGameObjectsActive(const std::vector<int>& instanceIDs, bool value) {
	auto objects = Object::FindObjectsByType<GameObject>(true);
	for (const auto& obj : objects) {
		if (!obj) continue;

		if (std::find(instanceIDs.begin(), instanceIDs.end(),
			static_cast<int>(obj->GetInstanceID())) != instanceIDs.end()) {
			obj->SetActive(value);
		}
	}
}

void GameObject::UpdateActiveInHierarchy() {
	bool parentActive = true;
	if (auto* parent = m_transform->GetParent()) {
		parentActive = parent->GetGameObject()->IsActiveInHierarchy();
	}

	const bool wasActive = m_activeInHierarchy;
	m_activeInHierarchy = m_activeSelf && parentActive;

	if (wasActive != m_activeInHierarchy) {
		HandleActivationChange(wasActive);
	}

	for (auto* child : m_transform->GetChildren()) {
		if (child) {
			child->GetGameObject()->UpdateActiveInHierarchy();
		}
	}
}

void GameObject::HandleActivationChange(bool wasActive) {
	const bool isActiveNow = m_activeInHierarchy;

	// If we're turning off, send OnDisable to behaviours that had been enabled.
	if (!isActiveNow) {
		for (const auto& component : m_components) {
			auto behaviour = std::dynamic_pointer_cast<MonoBehaviour>(component);
			if (!behaviour) continue;

			// Only disable if it was enabled in the first place.
			if (behaviour->HasOnEnableBeenCalled()) {
				behaviour->TriggerDisable();
			}
		}
		return;
	}

	// If we're turning on, queue lifecycle so Awake/OnEnable/Start run in scene order.
	for (const auto& component : m_components) {
		auto behaviour = std::dynamic_pointer_cast<MonoBehaviour>(component);
		if (!behaviour) continue;

		QueueLifecycle(behaviour.get());
	}
}

void GameObject::RegisterComponent(const std::shared_ptr<Component>& component) {
	if (!component) {
		return;
	}

	// Enforce rules:
	// - one Rigidbody2D max
	// - Collider2D requires Rigidbody2D
	if (std::dynamic_pointer_cast<Rigidbody2D>(component)) {
		if (GetComponent<Rigidbody2D>()) {
			THROW_ENGINE_EXCEPTION("GameObject '") << GetName() << "' already has a Rigidbody2D";
		}
	}
	if (std::dynamic_pointer_cast<Collider2D>(component)) {
		if (!GetComponent<Rigidbody2D>()) {
			THROW_ENGINE_EXCEPTION("GameObject '") << GetName()
				<< "' must have a Rigidbody2D before adding a Collider2D";
		}
	}

	m_components.push_back(component);
	Object::RegisterObject(component);

	// If we add a behaviour while active in hierarchy, queue its lifecycle.
	if (auto behaviour = std::dynamic_pointer_cast<MonoBehaviour>(component)) {
		QueueLifecycle(behaviour.get());
	}

	// Initialize physics components immediately (engine exception rules already applied above).
	if (auto rigidbody = std::dynamic_pointer_cast<Rigidbody2D>(component)) {
		rigidbody->Initialize();
	}

	if (auto collider = std::dynamic_pointer_cast<Collider2D>(component)) {
		collider->Initialize();
	}
}

void GameObject::UnregisterComponent(const std::shared_ptr<Component>& component) {
	auto it = std::find(m_components.begin(), m_components.end(), component);
	if (it != m_components.end()) {
		m_components.erase(it);
	}
}

void GameObject::RemoveComponent(const Component* component) {
	if (!component) {
		return;
	}
	auto it = std::remove_if(m_components.begin(), m_components.end(),
		[component](const std::shared_ptr<Component>& entry) { return entry.get() == component; });
	m_components.erase(it, m_components.end());
}

void GameObject::DestroyImmediateInternal() {
	if (IsDestroyed()) {
		return;
	}

	// Destroy children first.
	auto children = m_transform->GetChildren();
	for (auto* child : children) {
		if (!child) continue;

		auto* childObject = child->GetGameObject();
		childObject->DestroyImmediateInternal();
		childObject->MarkDestroyed();
	}

	// Destroy components.
	auto components = m_components;
	for (const auto& component : components) {
		if (!component) continue;

		component->DestroyImmediateInternal();
		Object::UnregisterObject(component->GetInstanceID());
	}
	m_components.clear();

	// Detach from parent.
	if (m_transform && m_transform->GetParent()) {
		m_transform->GetParent()->RemoveChild(m_transform.get());
	}

	// Remove from scene.
	if (m_scene) {
		m_scene->RemoveGameObject(this);
	}
}

void GameObject::QueueLifecycle(MonoBehaviour* behaviour) {
	if (!behaviour || !m_scene) {
		return;
	}
	m_scene->QueueLifecycle(behaviour);
}

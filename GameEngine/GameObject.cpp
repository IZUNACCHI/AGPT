#include "GameObject.h"
#include "Component.h"
#include "MonoBehaviour.h"
#include "Scene.h"
#include "Collider2D.h"
#include "Rigidbody2D.h"
#include "Transform.h"
#include <algorithm>

GameObject::GameObject(const std::string& name)
	: Object(name) {
	m_transform = std::make_shared<Transform>(this);
	RegisterComponent(m_transform);
}

GameObject::~GameObject() {
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
/*
void GameObject::SendMessage(const std::string& methodName) {
	for (const auto& component : m_components) {
		if (auto behaviour = std::dynamic_pointer_cast<MonoBehaviour>(component)) {
			behaviour->ReceiveMessage(methodName);
		}
	}
}

void GameObject::SendMessageUp(const std::string& methodName) {
	SendMessage(methodName);
	if (auto* parent = m_transform->GetParent()) {
		parent->GetGameObject()->SendMessageUp(methodName);
	}
}

void GameObject::SendMessageDown(const std::string& methodName) {
	SendMessage(methodName);
	for (auto* child : m_transform->GetChildren()) {
		if (child) {
			child->GetGameObject()->SendMessageDown(methodName);
		}
	}
}
*/

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

	for (const auto& component : m_components) {
		if (component.get() == m_transform.get()) {
			continue;
		}
		auto cloneComponent = component->Clone();
		if (!cloneComponent) {
			continue;
		}
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
		if (!obj) {
			continue;
		}
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

	bool wasActive = m_activeInHierarchy;
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

void GameObject::RegisterComponent(const std::shared_ptr<Component>& component) {
	if (!component) {
		return;
	}

	m_components.push_back(component);
	Object::RegisterObject(component);

	if (auto behaviour = std::dynamic_pointer_cast<MonoBehaviour>(component)) {
		QueueLifecycle(behaviour.get());
	}

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

	auto children = m_transform->GetChildren();
	for (auto* child : children) {
		if (child) {
			auto* childObject = child->GetGameObject();
			childObject->DestroyImmediateInternal();
			childObject->MarkDestroyed();
		}
	}

	auto components = m_components;
	for (const auto& component : components) {
		if (component) {
			component->DestroyImmediateInternal();
			Object::UnregisterObject(component->GetInstanceID());
		}
	}
	m_components.clear();

	if (m_transform && m_transform->GetParent()) {
		m_transform->GetParent()->RemoveChild(m_transform.get());
	}

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

void GameObject::HandleActivationChange(bool wasActive) {
	for (const auto& component : m_components) {
		auto behaviour = std::dynamic_pointer_cast<MonoBehaviour>(component);
		if (!behaviour) {
			continue;
		}
		if (!wasActive && m_activeInHierarchy) {
			QueueLifecycle(behaviour.get());
		}
		else if (wasActive && !m_activeInHierarchy) {
			behaviour->TriggerDisable();
		}
	}
}

#pragma once
#include <vector>
#include <memory>
#include <string>
#include <type_traits>
#include "Component.h"

class Scene;

//There's probably a better way to do memory management for GameObjects
class GameObject {

public:
	GameObject(const std::string& name = "GameObject", bool startActive = true, bool startVisible = true) : name(name), active(startActive), visible(startVisible) {}

	~GameObject() {
		//Detach from parent so parent doesn't hold dangling pointer
		RemoveFromParent();
		//Components will be automatically deleted due to unique_ptr
		components.clear();
		//Remove itself from children's parent pointers
		for(auto* child : children) {
			child->SetParent(nullptr);
		}
		
	}

	// Scene reference (not owning)
	Scene* owningScene = nullptr;

	// Hierarchy
	void SetParent(GameObject* newParent);
	GameObject* GetParent() const { return parent; }
	const std::vector<GameObject*>& GetChildren() const { return children; }

	// Components
	template <typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		auto comp = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = comp.get();
		ptr->owner = this;
		components.push_back(std::move(comp));
		ptr->Start();
		return ptr;
	}

	// Get first component of type T
	template <typename T>
	T* GetComponent() const {
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		for (const auto& comp : components) {
			if (auto* casted = dynamic_cast<T*>(comp.get())) {
				return casted;
			}
		}
		return nullptr;
	}

	// Get all components of type T
	template <typename T>
	std::vector<T*> GetComponents() const {
		// Ensure T is derived from Component
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		// Vector to hold results
		std::vector<T*> result;
		for (const auto& comp : components) {
			if (auto* casted = dynamic_cast<T*>(comp.get())) {
				result.push_back(casted);
			}
		}
		return result;
	}

	// Update traverses hierarchy
	void Update(float deltaTime);
	std::string name;
	bool active;
	bool visible;

	// Check if this GameObject is active in the hierarchy
	bool IsActive() const
	{
		// If no parent, return own active state
		if (!this->parent) return active;

		// Traverse up the hierarchy to check all parents
		const GameObject* curr = this;
		while (curr)
		{
			if (!curr->active) return false;
			curr = curr->parent;
		}
		return true;
	}

	// Reparent a child GameObject to this one
	void AddChild(GameObject* child);

	// detach a child GameObject
	void RemoveChild(GameObject* child);

	// Remove this object from its parent
	void RemoveFromParent();

private:
	GameObject* parent = nullptr; //Not Obligatory
	// Children pointers (non-owning)
	std::vector<GameObject*> children;
	// Components (owning)
	std::vector<std::unique_ptr<Component>> components;
};
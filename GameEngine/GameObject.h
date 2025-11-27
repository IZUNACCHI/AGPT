#pragma once
#include <vector>
#include <memory>
#include <string>
#include <type_traits>
#include "Component.h"

class GameObject {
public:
	GameObject(const std::string& name = "GameObject") : name(name) {}

	// Hierarchy
	void SetParent(GameObject* newParent);
	GameObject* GetParent() const { return parent; }
	const std::vector<std::unique_ptr<GameObject>>& GetChildren() const { return children; }
	GameObject* AddChild(const std::string& childName = "Child");

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

	template <typename T>
	std::vector<T*> GetComponents() const {
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		std::vector<T*> result;
		for (const auto& comp : components) {
			if (auto* casted = dynamic_cast<T*>(comp.get())) {
				result.push_back(casted);
			}
		}
		return result;
	}

	// Update traverses hierarchy
	void Update();

	std::string name;

private:
	GameObject* parent = nullptr; //Not Obligatory
	std::vector<std::unique_ptr<GameObject>> children;
	std::vector<std::unique_ptr<Component>> components;

	// Helper to remove from parent's children
	void RemoveFromParent();
};
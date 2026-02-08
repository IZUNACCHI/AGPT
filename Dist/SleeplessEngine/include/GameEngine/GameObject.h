#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include "Object.h"

class Scene;
class Component;
class MonoBehaviour;
class Transform;

// Root entity that owns Components and a Transform
class GameObject : public Object {
public:
	// Creates a GameObject with the provided name
	explicit GameObject(const std::string& name = "GameObject");
	// Destroys the GameObject
	~GameObject() override;

	// Returns the local active state
	bool IsActiveSelf() const { return m_activeSelf; }
	// Returns true if active in hierarchy
	bool IsActiveInHierarchy() const { return m_activeInHierarchy; }
	// Returns the layer value
	int GetLayer() const { return m_layer; }
	// Sets the layer value
	void SetLayer(int layer) { m_layer = layer; }
	// Returns the owning Scene
	Scene* GetScene() const { return m_scene; }
	// Returns the Transform component
	Transform* GetTransform() const { return m_transform.get(); }

	// Sets the active state for this GameObject
	void SetActive(bool value);

	// Adds a component of type T to this GameObject
	template<typename T, typename... Args>
	std::shared_ptr<T> AddComponent(Args&&... args);

	// Returns the first component of type T
	template<typename T>
	std::shared_ptr<T> GetComponent() const;

	// Returns all components of type T
	template<typename T>
	std::vector<std::shared_ptr<T> > GetComponents() const;

	// Returns the first component with the given component name
	std::shared_ptr<Component> GetComponentByName(const std::string& componentName) const;

	// Returns the first component of type T with the given component name
	template<typename T>
	std::shared_ptr<T> GetComponentByName(const std::string& componentName) const;

	// Returns the first component of type T in children
	template<typename T>
	std::shared_ptr<T> GetComponentInChildren() const;

	// Returns all components of type T in children
	template<typename T>
	std::vector<std::shared_ptr<T> > GetComponentsInChildren() const;

	// Returns the first component of type T in parents
	template<typename T>
	std::shared_ptr<T> GetComponentInParent() const;

	// Returns all components of type T in parents
	template<typename T>
	std::vector<std::shared_ptr<T> > GetComponentsInParent() const;

	// Returns the component index on this GameObject
	size_t GetComponentIndex(const Component* component) const;

	// Clones this GameObject and its Components
	std::shared_ptr<GameObject> Clone() const;

	// Finds a GameObject by name or path across scenes
	static std::shared_ptr<GameObject> Find(const std::string& nameOrPath);
	// Returns the Scene of a GameObject by instance ID
	static Scene* GetScene(int instanceID);
	// Sets active state for a list of instance IDs
	static void SetGameObjectsActive(const std::vector<int>& instanceIDs, bool value);

private:
	friend class Scene;
	friend class Component;
	friend class MonoBehaviour;
	friend class Transform;
	friend class Object;

	// Assigns the owning Scene.
	void SetScene(Scene* scene) { m_scene = scene; }
	// Recomputes active state for hierarchy
	void UpdateActiveInHierarchy();
	// Registers a component on this GameObject
	void RegisterComponent(const std::shared_ptr<Component>& component);
	// Unregisters a component from this GameObject
	void UnregisterComponent(const std::shared_ptr<Component>& component);
	// Removes a component by pointer
	void RemoveComponent(const Component* component);
	// Handles immediate destruction
	void DestroyImmediateInternal() override;

	// Queues a MonoBehaviour for lifecycle processing
	void QueueLifecycle(MonoBehaviour* behaviour);
	// Handles activation changes in hierarchy
	void HandleActivationChange(bool wasActive);

	bool m_activeSelf = true; // Local active state
	bool m_activeInHierarchy = true; // Active state in hierarchy
	int m_layer = 0; // Layer value for rendering
	Scene* m_scene = nullptr; // Owning Scene pointer

	std::shared_ptr<Transform> m_transform; // Transform component
	std::vector<std::shared_ptr<Component> > m_components; // Owned components
};

#include "GameObject.inl"
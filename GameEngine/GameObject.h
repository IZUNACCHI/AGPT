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

/// Root entity that owns Components and a Transform.
class GameObject : public Object {
public:
	/// Creates a GameObject with the provided name.
	explicit GameObject(const std::string& name = "GameObject");
	/// Destroys the GameObject.
	~GameObject() override;

	/// Returns the local active state.
	bool IsActiveSelf() const { return m_activeSelf; }
	/// Returns true if active in hierarchy.
	bool IsActiveInHierarchy() const { return m_activeInHierarchy; }
	/// Returns the layer value.
	int GetLayer() const { return m_layer; }
	/// Sets the layer value.
	void SetLayer(int layer) { m_layer = layer; }
	/// Returns the owning Scene.
	Scene* GetScene() const { return m_scene; }
	/// Returns the Transform component.
	Transform* GetTransform() const { return m_transform.get(); }

	/// Sets the active state for this GameObject.
	void SetActive(bool value);

	/// Adds a component of type T to this GameObject.
	template<typename T, typename... Args>
	std::shared_ptr<T> AddComponent(Args&&... args);

	/// Returns the first component of type T.
	template<typename T>
	std::shared_ptr<T> GetComponent() const;

	/// Returns all components of type T.
	template<typename T>
	std::vector<std::shared_ptr<T> > GetComponents() const;

	/// Returns the first component of type T in children.
	template<typename T>
	std::shared_ptr<T> GetComponentInChildren() const;

	/// Returns all components of type T in children.
	template<typename T>
	std::vector<std::shared_ptr<T> > GetComponentsInChildren() const;

	/// Returns the first component of type T in parents.
	template<typename T>
	std::shared_ptr<T> GetComponentInParent() const;

	/// Returns all components of type T in parents.
	template<typename T>
	std::vector<std::shared_ptr<T> > GetComponentsInParent() const;

	/// Returns the component index on this GameObject.
	size_t GetComponentIndex(const Component* component) const;

	/// Sends a message to all MonoBehaviours on this GameObject.
	void SendMessage(const std::string& methodName);
	/// Sends a message up the Transform hierarchy.
	void SendMessageUp(const std::string& methodName);
	/// Sends a message down the Transform hierarchy.
	void SendMessageDown(const std::string& methodName);

	/// Clones this GameObject and its Components.
	std::shared_ptr<GameObject> Clone() const;

	/// Finds a GameObject by name or path across scenes.
	static std::shared_ptr<GameObject> Find(const std::string& nameOrPath);
	/// Returns the Scene of a GameObject by instance ID.
	static Scene* GetScene(int instanceID);
	/// Sets active state for a list of instance IDs.
	static void SetGameObjectsActive(const std::vector<int>& instanceIDs, bool value);

private:
	/// Allows Scene to manage GameObjects.
	friend class Scene;
	/// Allows Component to access removal logic.
	friend class Component;
	/// Allows MonoBehaviour to access internals.
	friend class MonoBehaviour;
	/// Allows Transform to manage hierarchy.
	friend class Transform;
	/// Allows Object to manage destruction.
	friend class Object;

	/// Assigns the owning Scene.
	void SetScene(Scene* scene) { m_scene = scene; }
	/// Recomputes active state for hierarchy.
	void UpdateActiveInHierarchy();
	/// Registers a component on this GameObject.
	void RegisterComponent(const std::shared_ptr<Component>& component);
	/// Unregisters a component from this GameObject.
	void UnregisterComponent(const std::shared_ptr<Component>& component);
	/// Removes a component by pointer.
	void RemoveComponent(const Component* component);
	/// Handles immediate destruction.
	void DestroyImmediateInternal() override;

	/// Queues a MonoBehaviour for lifecycle processing.
	void QueueLifecycle(MonoBehaviour* behaviour);
	/// Handles activation changes in hierarchy.
	void HandleActivationChange(bool wasActive);

	/// Local active state.
	bool m_activeSelf = true;
	/// Cached active-in-hierarchy state.
	bool m_activeInHierarchy = true;
	/// Layer value.
	int m_layer = 0;
	/// Owning Scene pointer.
	Scene* m_scene = nullptr;

	/// Transform component.
	std::shared_ptr<Transform> m_transform;
	/// All attached components.
	std::vector<std::shared_ptr<Component> > m_components;
};

#include "GameObject.inl"

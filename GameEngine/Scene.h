#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <vector>
#include "GameObject.h"

class MonoBehaviour;

/// Container for GameObjects and lifecycle processing.
class Scene {
public:
	/// Creates a Scene with the given name.
	Scene(const std::string& name);
	/// Destroys the Scene.
	virtual ~Scene();

	/// Called when the Scene is created.
	virtual void OnCreate() {}
	/// Called when the Scene starts.
	virtual void OnStart() {}
	/// Called once per frame.
	virtual void OnUpdate(float) {}
	/// Called on a fixed timestep.
	virtual void OnFixedUpdate(float) {}
	/// Called after Update.
	virtual void OnLateUpdate(float) {}
	/// Called during render.
	virtual void OnRender() {}
	/// Called when the Scene is destroyed.
	virtual void OnDestroy() {}

	/// Creates and adopts a GameObject derived type.
	template<typename T, typename... Args>
	std::shared_ptr<T> CreateGameObject(const std::string& name = "GameObject", Args&&... args);

	/// Starts the Scene.
	void Start();
	/// Updates the Scene.
	void Update(float deltaTime);
	/// Updates the Scene on a fixed timestep.
	void FixedUpdate(float fixedDeltaTime);
	/// Updates the Scene after Update.
	void LateUpdate(float deltaTime);
	/// Renders the Scene.
	void Render();
	/// Unloads the Scene and destroys objects.
	void Unload();

	/// Returns the Scene name.
	const std::string& GetName() const { return m_name; }
	/// Returns whether the Scene is active.
	bool IsActive() const { return m_isActive; }

	/// Returns the root GameObjects for editing.
	std::vector<std::shared_ptr<GameObject>>& GetRootGameObjects() { return m_rootGameObjects; }
	/// Returns the root GameObjects for reading.
	const std::vector<std::shared_ptr<GameObject>>& GetRootGameObjects() const { return m_rootGameObjects; }

	/// Finds a GameObject by name or path across scenes.
	static std::shared_ptr<GameObject> FindGameObject(const std::string& nameOrPath);


private:
	/// Allows GameObject to call into Scene internals.
	friend class GameObject;
	/// Allows Transform to update root tracking when reparenting.
	friend class Transform;
	/// Allows MonoBehaviour to queue lifecycle work.
	friend class MonoBehaviour;

	/// Queues a MonoBehaviour for lifecycle processing.
	void QueueLifecycle(MonoBehaviour* behaviour);
	/// Processes queued lifecycle work.
	void ProcessLifecycleQueue();
	/// Adopts a new GameObject into the Scene.
	void AdoptGameObject(const std::shared_ptr<GameObject>& obj);
	/// Removes a GameObject from the Scene.
	void RemoveGameObject(GameObject* obj);
	/// Updates root object tracking for hierarchy changes.
	void UpdateRootGameObject(GameObject* obj);

	/// Scene name.
	std::string m_name;
	/// Whether the Scene is active.
	bool m_isActive = false;
	/// Whether the Scene is marked for unload.
	bool m_markedForUnload = false;

	/// Root GameObjects for the Scene.
	std::vector<std::shared_ptr<GameObject>> m_rootGameObjects;
	/// All GameObjects in the Scene.
	std::vector<std::shared_ptr<GameObject>> m_allGameObjects;
	/// Lookup map for GameObjects by instance ID.
	std::unordered_map<uint32_t, std::shared_ptr<GameObject>> m_gameObjectById;

	/// Pending lifecycle queue.
	std::vector<MonoBehaviour*> m_pendingLifecycle;

	/// Registry of all active Scenes.
	static std::vector<Scene*> s_scenes;
};

template<typename T, typename... Args>
std::shared_ptr<T> Scene::CreateGameObject(const std::string& name, Args&&... args) {
	static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");
	auto obj = std::make_shared<T>(name, std::forward<Args>(args)...);
	Object::RegisterObject(obj);
	AdoptGameObject(obj);
	return obj;
}

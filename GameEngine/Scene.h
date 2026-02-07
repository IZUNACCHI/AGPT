#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <utility>
#include <vector>
#include "GameObject.h"
#include "ObjectPool.h"
#include "GameMode.h"

class MonoBehaviour;

// Represents a level, contains GameObjects and manages their lifecycle
class Scene {
public:
	// Creates a Scene with the given name
	Scene(const std::string& name);
	// Destroys the Scene
	virtual ~Scene();

	// Called when the Scene is created
	virtual void OnCreate() {}
	// Called when the Scene starts
	virtual void OnStart() {}
	// Called once per frame
	virtual void OnUpdate() {}
	// Called on a fixed timestep
	virtual void OnFixedUpdate() {}
	// Called after Update
	virtual void OnLateUpdate() {}
	// Called during render
	virtual void OnRender() {}
	// Called when the Scene is destroyed
	virtual void OnDestroy() {}

	// Creates and adopts a GameObject derived type
	template<typename T, typename... Args>
	std::shared_ptr<T> CreateGameObject(const std::string& name = "GameObject", Args&&... args);

	// Starts the Scene
	void Start();
	// Updates the Scene
	void Update();
	// Updates the Scene on a fixed timestep
	void FixedUpdate();
	// Updates the Scene after Update
	void LateUpdate();
	// Renders the Scene
	void Render();
	// Unloads the Scene and destroys objects
	void Unload();

	
	// Returns the Scene's GameMode
	GameMode* GetGameMode() const { return m_gameMode.get(); }

	// Replaces the Scene's GameMode (cannot be null; will fallback to EmptyGameMode)
	void SetGameMode(std::unique_ptr<GameMode> mode);

	// Convenience helper to construct and set a GameMode in-place
	template<typename TMode, typename... Args>
	TMode* SetGameMode(Args&&... args) {
		auto mode = std::make_unique<TMode>(std::forward<Args>(args)...);
		TMode* ptr = mode.get();
		SetGameMode(std::move(mode));
		return ptr;
	}

	// Returns the Scene name.
	const std::string& GetName() const { return m_name; }
	// Returns whether the Scene is active
	bool IsActive() const { return m_isActive; }

	// Returns the root GameObjects for editing
	std::vector<std::shared_ptr<GameObject>>& GetRootGameObjects() { return m_rootGameObjects; }
	// Returns the root GameObjects for reading
	const std::vector<std::shared_ptr<GameObject>>& GetRootGameObjects() const { return m_rootGameObjects; }
	// Object pooling (per-scene)
	ObjectPool& GetObjectPool() { return m_objectPool; }
	const ObjectPool& GetObjectPool() const { return m_objectPool; }

	// Convenience pooled spawn: reuse inactive objects when available
	template<typename T, typename... Args>
	std::shared_ptr<T> CreateGameObjectPooled(const std::string& poolKey, const std::string& name = "GameObject", Args&&... args);
	// Release a GameObject into a pool (deactivates instead of destroying)
	void ReleaseGameObjectToPool(const std::string& poolKey, GameObject* obj);


	// Finds a GameObject by name or path across scenes
	static std::shared_ptr<GameObject> FindGameObject(const std::string& nameOrPath);


private:
	// Allows GameObject to call into Scene internals
	friend class GameObject;
	// Allows Transform to update root tracking when reparenting
	friend class Transform;
	// Allows MonoBehaviour to queue lifecycle work
	friend class MonoBehaviour;

	// Queues a MonoBehaviour for lifecycle processing
	void QueueLifecycle(MonoBehaviour* behaviour);

	// Ensures the Scene always has a valid GameMode instance
	// If one wasn't provided by the game layer, an EmptyGameMode is created
	void EnsureGameMode();
	// Processes queued lifecycle work
	void ProcessLifecycleQueue();
	// Queues a new GameObject to be adopted when lifecycle is processed
	void QueueAdoptGameObject(const std::shared_ptr<GameObject>& obj);
	// Flushes queued GameObjects into scene-owned collections
	// Returns true if any objects were adopted
	bool ProcessPendingAdopts();
	// Queues a new GameObject to be adopted into the Scene
	// This is intentionally deferred so we never mutate the scene lists while iterating
	void AdoptGameObject(const std::shared_ptr<GameObject>& obj);
	// Immediately adopts a GameObject into the Scene-owned collections
	void AdoptGameObjectImmediate(const std::shared_ptr<GameObject>& obj);
	// Removes a GameObject from the Scene
	void RemoveGameObject(GameObject* obj);
	// Updates root object tracking for hierarchy changes
	void UpdateRootGameObject(GameObject* obj);

	// Scene name
	std::string m_name;
	// Whether the Scene is active
	bool m_isActive = false;
	// Whether the Scene is marked for unload
	bool m_markedForUnload = false;

	// Root GameObjects for the Scene
	std::vector<std::shared_ptr<GameObject>> m_rootGameObjects;
	// All GameObjects in the Scene
	std::vector<std::shared_ptr<GameObject>> m_allGameObjects;
	// Lookup map for GameObjects by instance ID
	std::unordered_map<uint32_t, std::shared_ptr<GameObject>> m_gameObjectById;

	// Pending lifecycle queue
	std::vector<MonoBehaviour*> m_pendingLifecycle;

	// Pending GameObjects waiting to be adopted into the scene
	std::vector<std::shared_ptr<GameObject>> m_pendingAdopt;
	// Pooled (inactive) objects stored by user-defined keys
	ObjectPool m_objectPool;

	// Per-scene mode that owns shared gameplay rules for this level
	std::unique_ptr<GameMode> m_gameMode;


	// Registry of all active Scenes
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

template<typename T, typename... Args>
std::shared_ptr<T> Scene::CreateGameObjectPooled(const std::string& poolKey, const std::string& name, Args&&... args) {
	static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");
	return m_objectPool.Acquire<T>(poolKey, [&]() {
		return CreateGameObject<T>(name, std::forward<Args>(args)...);
	});
}

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <array>
#include "GameObject.h"
#include "Types.hpp"

class Scene {
public:
	static const int MAX_LAYERS = 32;

	Scene(const std::string& name);
	virtual ~Scene();

	// Lifecycle methods
	virtual void OnCreate() {}     // Called when scene is loaded
	virtual void OnStart() {}      // Called when scene becomes active
	virtual void OnUpdate(float deltaTime) {}
	virtual void OnFixedUpdate(float fixedDeltaTime) {}
	virtual void OnLateUpdate(float deltaTime) {}
	virtual void OnRender() {}
	virtual void OnDestroy() {}    // Called before scene is unloaded

	// GameObject management
	std::shared_ptr<GameObject> CreateGameObject(const std::string& name = "GameObject");
	std::shared_ptr<GameObject> CreateGameObject(const std::string& name, Transform* parent);

	void DestroyGameObject(std::shared_ptr<GameObject> obj);
	void DestroyGameObjectImmediate(std::shared_ptr<GameObject> obj);

	// Scene operations
	void Start();        // Activate the scene
	void Update(float deltaTime);
	void FixedUpdate(float fixedDeltaTime);
	void LateUpdate(float deltaTime);
	void Render();
	void Unload();       // Mark scene for unloading
	void Clear();        // Clear all game objects

	// Getters
	const std::string& GetName() const { return m_name; }
	bool IsActive() const { return m_isActive; }
	bool IsMarkedForUnload() const { return m_markedForUnload; }

	// GameObject access
	std::vector<std::shared_ptr<GameObject>>& GetRootGameObjects() { return m_rootGameObjects; }
	const std::vector<std::shared_ptr<GameObject>>& GetRootGameObjects() const { return m_rootGameObjects; }

	// Find game objects
	std::shared_ptr<GameObject> FindGameObjectByName(const std::string& name);
	std::vector<std::shared_ptr<GameObject>> FindGameObjectsByTag(const std::string& tag);
	std::shared_ptr<GameObject> FindGameObjectByID(uint32_t id);

	// Layer management (similar to Unity's layers)
	void SetLayerName(int layerIndex, const std::string& layerName);
	const std::string& GetLayerName(int layerIndex) const;
	int GetLayerIndex(const std::string& layerName) const;

	// Render order based on layers
	void SetRenderOrderForLayer(int layerIndex, int order);
	int GetRenderOrderForLayer(int layerIndex) const;

private:
	void ProcessDestructionQueue();
	void AddToUpdateList(std::shared_ptr<GameObject> obj);
	void AddToRenderList(std::shared_ptr<Component> component);
	void RemoveFromUpdateList(std::shared_ptr<GameObject> obj);
	void RemoveFromRenderList(std::shared_ptr<Component> component);

	friend class GameObject;
	friend class Component;
	friend class Behaviour;
	friend class RenderableComponent;

	std::string m_name;
	bool m_isActive = false;
	bool m_markedForUnload = false;

	// GameObject management
	std::vector<std::shared_ptr<GameObject>> m_rootGameObjects;
	std::vector<std::shared_ptr<GameObject>> m_allGameObjects;
	std::unordered_map<uint32_t, std::shared_ptr<GameObject>> m_gameObjectByID;

	// Update lists (separated for efficiency)
	std::vector<std::shared_ptr<GameObject>> m_activeGameObjects;      // For Update
	std::vector<std::shared_ptr<GameObject>> m_fixedUpdateObjects;     // For FixedUpdate
	std::vector<std::shared_ptr<GameObject>> m_lateUpdateObjects;      // For LateUpdate

	// Render list (components with Draw method)
	std::vector<std::shared_ptr<Component>> m_renderComponents;

	// Destruction queue
	std::vector<std::shared_ptr<GameObject>> m_destructionQueue;

	// Layer management (32 layers like Unity)
	std::array<std::string, MAX_LAYERS> m_layerNames;
	std::array<int, MAX_LAYERS> m_layerRenderOrders;
	std::unordered_map<std::string, int> m_layerNameToIndex;

	// Object counter for unique IDs
	static uint32_t s_nextGameObjectID;
};
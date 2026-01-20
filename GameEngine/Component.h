#pragma once

#include <string>
#include "Object.h"

class GameObject;
class Scene;

// Base class for all components attached to a GameObject.
class Component : public Object {
public:
	explicit Component(std::string name = "");
	virtual ~Component() = default;

	// Lifecycle callbacks (default no-op).
	virtual void Awake() {}
	virtual void Init() {}
	virtual void OnEnable() {}
	virtual void OnCreate() {}
	virtual void FixedUpdate(float) {}
	virtual void Update(float) {}
	virtual void LateUpdate(float) {}
	virtual void OnDisable() {}
	virtual void OnDestroy() {}

	void SetEnabled(bool enabled);
	bool IsEnabled() const { return m_enabled; }

	GameObject* GetGameObject() const { return m_gameObject; }
	Scene* GetScene() const;

	bool WasEnabledInHierarchy() const { return m_effectivelyEnabled; }

protected:
	bool IsEffectivelyEnabled() const;

private:
	friend class GameObject;
	friend class Scene;

	void SetGameObject(GameObject* owner) { m_gameObject = owner; }
	void CallAwakeInit();
	void CallOnCreate();
	void OnActiveInHierarchyChanged(bool nowActive);
	void MarkPendingDestroyInternal();

	bool m_enabled = true;
	bool m_effectivelyEnabled = false;
	bool m_awakeCalled = false;
	bool m_createCalled = false;
	bool m_destroyCalled = false;
	GameObject* m_gameObject = nullptr;
};
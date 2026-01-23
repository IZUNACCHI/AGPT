#pragma once

#include <functional>
#include <string>
#include <vector>
#include "Behaviour.h"

class Collider2D;

/// Script base class with Unity-style lifecycle callbacks.
class MonoBehaviour : public Behaviour {
public:
	/// Creates a MonoBehaviour.
	MonoBehaviour();
	/// Destroys the MonoBehaviour.
	~MonoBehaviour() override = default;

	/// Returns true if the object is queued for destruction.
	bool IsMarkedForDestruction() const { return Object::IsMarkedForDestruction(); }
	/// Returns true if Awake has been called.
	bool DidAwake() const { return m_didAwake; }
	/// Returns true if Start has been called.
	bool DidStart() const { return m_didStart; }

	/// Called when the script instance is being loaded.
	virtual void Awake() {}
	/// Called before the first Update if the script is enabled.
	virtual void Start() {}
	/// Called once per frame while enabled and active.
	virtual void Update() {}
	/// Called at fixed time steps.
	virtual void FixedUpdate() {}
	/// Called after all Update calls.
	virtual void LateUpdate() {}
	/// Called when the behaviour becomes enabled and active.
	virtual void OnEnable() {}
	/// Called when the behaviour becomes disabled or inactive.
	virtual void OnDisable() {}
	/// Called when the behaviour is destroyed.
	virtual void OnDestroy() {}

	/// Called when a collision begins.
	virtual void OnCollisionEnter(Collider2D* other) {}
	/// Called while a collision stays.
	virtual void OnCollisionStay(Collider2D* other) {}
	/// Called when a collision ends.
	virtual void OnCollisionExit(Collider2D* other) {}
	/// Called when a trigger begins.
	virtual void OnTriggerEnter(Collider2D* other) {}
	/// Called while a trigger stays.
	virtual void OnTriggerStay(Collider2D* other) {}
	/// Called when a trigger ends.
	virtual void OnTriggerExit(Collider2D* other) {}

	/// Restores default values.
	virtual void Reset() {}

	/// Invokes a method by name after a delay.
	void Invoke(const std::string& methodName, float time);
	/// Invokes a method repeatedly after an initial delay.
	void InvokeRepeating(const std::string& methodName, float delay, float rate);
	/// Cancels a pending invoke.
	void CancelInvoke(const std::string& methodName = "");
	/// Returns true if a method is currently scheduled.
	bool IsInvoking(const std::string& methodName) const;
	/*
	/// Receives a SendMessage call.
	virtual void ReceiveMessage(const std::string& methodName);
	*/

protected:
	/// Returns whether OnEnable has been called.
	bool HasOnEnableBeenCalled() const override;
	/// Responds to enabled state changes.
	void OnEnabledStateChanged(bool enabled) override;

private:
	/// Allows Scene to drive lifecycle.
	friend class Scene;
	/// Allows GameObject to control lifecycle.
	friend class GameObject;
	/// Allows Object to mark destruction.
	friend class Object;
	/// Allows Component to access lifecycle internals.
	friend class Component;

	/// Stores an invoke request for delayed calls.
	struct InvokeRequest {
		/// Method name to invoke.
		std::string methodName;
		/// Next scheduled time for invocation.
		float nextTime = 0.0f;
		/// Repeating interval in seconds.
		float rate = 0.0f;
		/// Whether the invoke repeats.
		bool repeating = false;
	};

	/// Triggers Awake if needed.
	void TriggerAwake();
	/// Triggers OnEnable if needed.
	void TriggerEnable();
	/// Triggers OnDisable if needed.
	void TriggerDisable();
	/// Triggers Start if needed.
	void TriggerStart();
	/// Triggers OnDestroy if needed.
	void TriggerDestroy();
	/// Processes scheduled invokes for this frame.
	void TickInvokes(float now);

	/// Whether Awake has been called.
	bool m_didAwake = false;
	/// Whether Start has been called.
	bool m_didStart = false;
	/// Whether OnEnable has been called.
	bool m_onEnableCalled = false;
	/// Whether the behaviour has ever been active.
	bool m_hasEverBeenActive = false;
	/// Whether destroy callbacks have been sent.
	bool m_destroyCallbacksSent = false;
	/// Pending invoke requests.
	std::vector<InvokeRequest> m_invokes;
};

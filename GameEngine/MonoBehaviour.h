#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "Behaviour.h"

class Collider2D;

class MonoBehaviour : public Behaviour {
public:
	using InvokeHandle = std::uint64_t;

	// Controls when an invoke advances time.
	enum class InvokeTickPolicy : std::uint8_t {
		WhileGameObjectActive,   // Advances while the GameObject is active in hierarchy
		WhileBehaviourEnabled    // Advances only while the behaviour is enabled
	};

	// Creates a MonoBehaviour.
	MonoBehaviour();
	// Destroys the MonoBehaviour.
	~MonoBehaviour() override = default;

	// Returns true if the object is queued for destruction.
	bool IsMarkedForDestruction() const { return Object::IsMarkedForDestruction(); }
	// Returns true if Awake has been called.
	bool DidAwake() const { return m_didAwake; }
	// Returns true if Start has been called.
	bool DidStart() const { return m_didStart; }

	// ---------------------------------------------------------------------
	// Engine dispatch entry points
	//
	// These are public so engine systems (Scene, Physics, etc.) can invoke
	// user callbacks without requiring the callbacks themselves to be public.
	//
	// User code should NOT call these directly. Override the protected
	// callback methods instead (Awake/Start/Update/OnTriggerEnter/etc.).
	// ---------------------------------------------------------------------

	/// Engine-only: dispatches Update().
	void InternalUpdate() { Update(); }
	/// Engine-only: advances Invoke/InvokeRepeating timers.
	void InternalTickInvokes(float now) { TickInvokes(now); }
	/// Engine-only: dispatches FixedUpdate().
	void InternalFixedUpdate() { FixedUpdate(); }
	/// Engine-only: dispatches LateUpdate().
	void InternalLateUpdate() { LateUpdate(); }

	void InternalOnCollisionEnter(Collider2D* other) { OnCollisionEnter(other); }
	void InternalOnCollisionStay(Collider2D* other) { OnCollisionStay(other); }
	void InternalOnCollisionExit(Collider2D* other) { OnCollisionExit(other); }
	void InternalOnTriggerEnter(Collider2D* other) { OnTriggerEnter(other); }
	void InternalOnTriggerStay(Collider2D* other) { OnTriggerStay(other); }
	void InternalOnTriggerExit(Collider2D* other) { OnTriggerExit(other); }


	/// Engine-only: dispatches Reset().
	void InternalReset() { Reset(); }

	// Schedules a function to run after a delay.
	InvokeHandle Invoke(std::function<void()> fn, float delaySeconds,
		InvokeTickPolicy policy = InvokeTickPolicy::WhileGameObjectActive);

	// Schedules a function to run repeatedly.
	InvokeHandle InvokeRepeating(std::function<void()> fn, float delaySeconds, float rateSeconds,
		InvokeTickPolicy policy = InvokeTickPolicy::WhileGameObjectActive);

	// Cancels a scheduled invoke.
	void CancelInvoke(InvokeHandle handle);

	// Returns true if the handle is still scheduled.
	bool IsInvoking(InvokeHandle handle) const;

	// Pauses a scheduled invoke without removing it.
	bool PauseInvoke(InvokeHandle handle);

	// Resumes a paused invoke.
	bool ResumeInvoke(InvokeHandle handle);

	// Restarts the invoke countdown.
	bool RestartInvoke(InvokeHandle handle);

	// Pauses all scheduled invokes on this behaviour.
	void PauseAllInvokes();

	// Resumes all paused invokes on this behaviour.
	void ResumeAllInvokes();

	// Schedules a method by name after a delay.
	void Invoke(const std::string& methodName, float time);

	// Schedules a method by name repeatedly.
	void InvokeRepeating(const std::string& methodName, float delay, float rate);

	// Cancels scheduled invokes by name (or all if empty).
	void CancelInvoke(const std::string& methodName = "");

	// Returns true if a method name is scheduled.
	bool IsInvoking(const std::string& methodName) const;

	// Registers a handler for a method name.
	void RegisterInvokeHandler(const std::string& methodName, std::function<void()> handler);

	// Removes a handler for a method name.
	void UnregisterInvokeHandler(const std::string& methodName);

	// Registers / unregisters invoke handlers by name.

protected:
	// ---------------------------------------------------------------------
	// User-overridable callbacks (keep these protected)
	// ---------------------------------------------------------------------

	// Restores default values for the component.
	virtual void Reset() {}
	
	// Called once when the component is initialized.
	virtual void Awake() {}
	// Called once before the first Update (if enabled).
	virtual void Start() {}
	// Called every frame while enabled.
	virtual void Update() {}
	// Called at fixed time steps while enabled.
	virtual void FixedUpdate() {}
	// Called after Update while enabled.
	virtual void LateUpdate() {}
	// Called when the behaviour becomes enabled.
	virtual void OnEnable() {}
	// Called when the behaviour becomes disabled.
	virtual void OnDisable() {}
	// Called when the object is being destroyed.
	virtual void OnDestroy() {}

	// Called when a collision begins.
	virtual void OnCollisionEnter(Collider2D* other) {}
	// Called while a collision continues.
	virtual void OnCollisionStay(Collider2D* other) {}
	// Called when a collision ends.
	virtual void OnCollisionExit(Collider2D* other) {}
	// Called when a trigger begins.
	virtual void OnTriggerEnter(Collider2D* other) {}
	// Called while a trigger continues.
	virtual void OnTriggerStay(Collider2D* other) {}
	// Called when a trigger ends.
	virtual void OnTriggerExit(Collider2D* other) {}

	// Receives a string invoke call.
	virtual void ReceiveMessage(const std::string& methodName);

	// Returns whether OnEnable has been called.
	bool HasOnEnableBeenCalled() const override;

	// Called when the enabled flag changes.
	void OnEnabledStateChanged(bool enabled) override;

private:
	friend class Scene;
	friend class GameObject;
	friend class Object;
	friend class Component;

	// Internal invoke request structure.
	struct InvokeRequest {
		// Unique invoke handle.
		InvokeHandle id = 0;

		// Optional method name to invoke.
		std::string methodName;
		// Optional function to invoke.
		std::function<void()> fn;

		// When to tick the invoke.
		InvokeTickPolicy policy = InvokeTickPolicy::WhileGameObjectActive;

		// Next scheduled time.
		float nextTime = 0.0f;
		// Repeating rate (0 = one-shot).
		float rate = 0.0f;
		// Whether the invoke is repeating.
		bool repeating = false;

		// Whether the invoke has been cancelled.
		bool cancelled = false;
		// Whether the invoke is currently paused.
		bool paused = false;
		// Remaining time when paused.
		float pausedRemaining = 0.0f;
	};

	// Runs Awake if needed.
	void TriggerAwake();

	// Runs OnEnable if needed.
	void TriggerEnable();

	// Runs OnDisable if needed.
	void TriggerDisable();

	// Runs Start if needed.
	void TriggerStart();

	// Runs OnDestroy if needed.
	void TriggerDestroy();

	// Executes due invokes.
	void TickInvokes(float now);

	// Generates a new invoke handle.
	InvokeHandle MakeInvokeId();

	// Applies invoke pausing rules when enabled state changes.
	void OnBehaviourEnabledChangedForInvokes(bool enabled);

	// Advances repeating invokes without accumulating drift.
	void AdvanceRepeating(InvokeRequest& req, float now);

	// Lifecycle tracking
	// Whether Awake has been called.
	bool m_didAwake = false;
	// Whether Start has been called.
	bool m_didStart = false;
	// Whether OnEnable has been called.
	bool m_onEnableCalled = false;
	// Whether the behaviour has ever been active.
	bool m_hasEverBeenActive = false;
	// Whether destroy callbacks have been sent.
	bool m_destroyCallbacksSent = false;

	std::vector<InvokeRequest> m_invokes;
	std::unordered_map<std::string, std::function<void()>> m_invokeHandlers;

	InvokeHandle m_nextInvokeId = 1;
};
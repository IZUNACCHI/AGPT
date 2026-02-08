#pragma once

class Scene;

// Per-scene rule container (spawning rules, win/lose conditions, scoring, etc.)
// A Scene always owns exactly one GameMode. If a Scene is created without explicitly
// setting one, the engine assigns a default "empty" mode that does nothing.
class GameMode {
public:
	GameMode() = default;
	virtual ~GameMode() = default;

	// Called when the GameMode is attached to a Scene
	virtual void OnAttach(Scene& scene) { (void)scene; }

	// Called when the Scene starts running
	virtual void OnStart() {}

	// Called once per frame before Scene / behaviours update
	virtual void OnUpdate() {}

	// Called on a fixed timestep before Scene / behaviours fixed update
	virtual void OnFixedUpdate() {}

	// Called after Update before Scene / behaviours late update
	virtual void OnLateUpdate() {}

	// Called during render before Scene render hook
	virtual void OnRender() {}

	// Called when the Scene is unloading
	virtual void OnDestroy() {}

	// Optional helper name for debugging/logging
	virtual const char* GetDebugName() const { return "GameMode"; }
};

// Default do-nothing mode used when a Scene doesn't provide one.
class EmptyGameMode final : public GameMode {
public:
	const char* GetDebugName() const override { return "EmptyGameMode"; }
};

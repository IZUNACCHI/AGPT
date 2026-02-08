#pragma once

#include <string>

/// Global, long-lived game object created when the engine starts and destroyed on shutdown.
class GameInstance {
public:
	GameInstance() = default;
	virtual ~GameInstance() = default;

	/// Called once after the engine has initialized its core systems
	virtual void OnInit() {}

	/// Called once right before the engine shuts down
	virtual void OnShutdown() {}

	virtual const char* GetDebugName() const { return "GameInstance"; }
};

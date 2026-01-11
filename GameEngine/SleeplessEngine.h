#pragma once
#include <memory>

class SleeplessEngine
{
public:
	struct InitParams
	{
		std::string title = "Sleepless Engine Game";
		int width = 1280;
		int height = 720;
		Scene* startingScene = nullptr;
	};

	static void Start(const InitParams& params);
	static void Run();                     // never returns until quit
	static void LoadScene(Scene* newScene); // call from game code anytime

	// Accessors
	static Renderer& GetRenderer();
	//static b2World& GetPhysicsWorld();
	static Scene* GetCurrentScene();

private:
	struct Impl;
	static inline std::unique_ptr<Impl> impl;
};
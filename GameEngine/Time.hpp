#pragma once
#include <memory>

class Time {
public:
	static void Initialize();
	static void Shutdown();

	static void Update();

	static float DeltaTime();
	static float TotalTime();
	static unsigned long FrameCount();

private:
	struct Impl;
	static std::unique_ptr<Impl> impl;
};
#pragma once

class Time {
public:
	void Update();

	float DeltaTime() const { return deltaTime; }
	float TotalTime() const { return totalTime; }
	unsigned long FrameCount() const { return frameCount; }

private:
	float deltaTime = 0.f;
	float totalTime = 0.f;
	unsigned long frameCount = 0;


	unsigned long long lastTicks = 0;
};
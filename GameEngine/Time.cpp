#include "Time.hpp"
#include <SDL3/SDL.h>

struct Time::Impl {
	
	//time since last frame
	float deltaTime = 0.f;
	//time since start
	float totalTime = 0.f;
	//frame count since start
	unsigned long frameCount = 0;
	//last tick count
	unsigned long long lastTicks = 0;
};

std::unique_ptr<Time::Impl> Time::impl = nullptr;

void Time::Initialize() {
	impl = std::make_unique<Impl>();
}

void Time::Shutdown() {
	impl.reset();
}

void Time::Update() {
	unsigned long long now = SDL_GetTicks();
	if (impl->lastTicks == 0)
		impl->lastTicks = now;

	impl->deltaTime = (now - impl->lastTicks) / 1000.0f;
	impl->lastTicks = now;

	impl->totalTime += impl->deltaTime;
	impl->frameCount++;
}

float Time::DeltaTime() { return impl ? impl->deltaTime : 0.f; }
float Time::TotalTime() { return impl ? impl->totalTime : 0.f; }
unsigned long Time::FrameCount() { return impl ? impl->frameCount : 0; }
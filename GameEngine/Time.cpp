#include "Time.hpp"
#include <SDL3/SDL.h>

void Time::Update() {
	unsigned long long now = SDL_GetTicks();
	if (lastTicks == 0)
		lastTicks = now;

	deltaTime = (now - lastTicks) / 1000.0f;
	lastTicks = now;

	totalTime += deltaTime;
	frameCount++;
}
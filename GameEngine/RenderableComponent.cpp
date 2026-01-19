#include "RenderableComponent.h"
#include "SleeplessEngine.h"

Renderer* RenderableComponent::GetRenderer() {
	// Route rendering access through the engine singleton.
	return SleeplessEngine::GetInstance().GetRenderer();
}
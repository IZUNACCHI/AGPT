#include "RenderableComponent.h"

#include "RenderSystem.h"

void RenderableComponent::DestroyImmediateInternal() {
	// If this component is registered as a renderable, remove it before the
	// GameObject drops its shared_ptr to the component.
	RenderSystem::Get().Unregister(this);

	// Continue with normal component destruction.
	Component::DestroyImmediateInternal();
}

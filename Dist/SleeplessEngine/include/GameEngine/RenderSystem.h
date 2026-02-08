#pragma once

#include <vector>

class RenderQueue;
class RenderableComponent;
class SpriteRenderer;
class TextRenderer;

// Centralized render system that tracks all renderable components
class RenderSystem {
public:
	static RenderSystem& Get();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	// Register/unregister a renderable component
	void Register(RenderableComponent* renderable);
	void Unregister(RenderableComponent* renderable);

	// Builds a render queue from all registered renderables
	void BuildQueue(RenderQueue& outQueue) const;

	// Clears all registered renderables.
	void Clear();

private:
	RenderSystem() = default;

	static void RemovePtr(std::vector<SpriteRenderer*>& v, SpriteRenderer* p);
	static void RemovePtr(std::vector<TextRenderer*>& v, TextRenderer* p);

	std::vector<SpriteRenderer*> m_sprites; // Registered SpriteRenderers
	std::vector<TextRenderer*> m_texts; // Registered TextRenderers
};

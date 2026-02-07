#include "RenderSystem.h"

#include "RenderQueue.h"
#include "RenderableComponent.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "GameObject.h"

#include <algorithm>

RenderSystem& RenderSystem::Get() {
	static RenderSystem instance;
	return instance;
}

void RenderSystem::RemovePtr(std::vector<SpriteRenderer*>& v, SpriteRenderer* p) {
	auto it = std::find(v.begin(), v.end(), p);
	if (it != v.end()) {
		*it = v.back();
		v.pop_back();
	}
}

void RenderSystem::RemovePtr(std::vector<TextRenderer*>& v, TextRenderer* p) {
	auto it = std::find(v.begin(), v.end(), p);
	if (it != v.end()) {
		*it = v.back();
		v.pop_back();
	}
}

void RenderSystem::Register(RenderableComponent* renderable) {
	if (!renderable) {
		return;
	}

	// Note: we only keep raw pointers; ownership stays with the GameObject/component system.
	if (auto* sprite = dynamic_cast<SpriteRenderer*>(renderable)) {
		if (std::find(m_sprites.begin(), m_sprites.end(), sprite) == m_sprites.end()) {
			m_sprites.push_back(sprite);
		}
		return;
	}
	if (auto* text = dynamic_cast<TextRenderer*>(renderable)) {
		if (std::find(m_texts.begin(), m_texts.end(), text) == m_texts.end()) {
			m_texts.push_back(text);
		}
		return;
	}
}

void RenderSystem::Unregister(RenderableComponent* renderable) {
	if (!renderable) {
		return;
	}
	if (auto* sprite = dynamic_cast<SpriteRenderer*>(renderable)) {
		RemovePtr(m_sprites, sprite);
		return;
	}
	if (auto* text = dynamic_cast<TextRenderer*>(renderable)) {
		RemovePtr(m_texts, text);
		return;
	}
}

void RenderSystem::BuildQueue(RenderQueue& outQueue) const {
	// NOTE: We filter here to keep the registry stable even if objects are toggled active.
	for (const auto* sprite : m_sprites) {
		if (!sprite) continue;
		if (!sprite->IsVisible()) continue;
		auto* go = sprite->GetGameObject();
		if (!go || !go->IsActiveInHierarchy()) continue;
		outQueue.Add(sprite);
	}

	for (const auto* text : m_texts) {
		if (!text) continue;
		if (!text->IsVisible()) continue;
		auto* go = text->GetGameObject();
		if (!go || !go->IsActiveInHierarchy()) continue;
		outQueue.Add(text);
	}
}

void RenderSystem::Clear() {
	m_sprites.clear();
	m_texts.clear();
}

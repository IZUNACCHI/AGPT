#include "SpriteRenderer.h"
#include "GameObject.h"
#include "Object.h"
#include "Renderer.h"
#include "Transform.h"
#include <algorithm>

SpriteRenderer::SpriteRenderer()
	: Component("SpriteRenderer") {
}

SpriteRenderer::SpriteRenderer(Texture* texture)
	: Component("SpriteRenderer"), m_texture(texture) {
	if (m_texture) {
		m_frameSize = m_texture->GetSize();
	}
}

void SpriteRenderer::SetTexture(Texture* texture) {
	m_texture = texture;
	if (m_texture && (m_frameSize.x <= 0 || m_frameSize.y <= 0)) {
		m_frameSize = m_texture->GetSize();
	}
}

void SpriteRenderer::SetFrameSize(const Vector2i& size) {
	m_frameSize = size;
}

void SpriteRenderer::SetFrameIndex(int index) {
	m_frameIndex = index;
}

Vector2i SpriteRenderer::GetResolvedFrameSize() const {
	if (m_frameSize.x > 0 && m_frameSize.y > 0) {
		return m_frameSize;
	}
	if (m_texture) {
		return m_texture->GetSize();
	}
	return Vector2i::Zero();
}

int SpriteRenderer::GetMaxFrames(const Vector2i& frameSize) const {
	if (!m_texture || frameSize.x <= 0 || frameSize.y <= 0) {
		return 0;
	}
	const Vector2i textureSize = m_texture->GetSize();
	const int columns = textureSize.x / frameSize.x;
	const int rows = textureSize.y / frameSize.y;
	if (columns <= 0 || rows <= 0) {
		return 0;
	}
	return columns * rows;
}

void SpriteRenderer::RenderAll(Renderer& renderer) {
	auto sprites = Object::FindObjectsByType<SpriteRenderer>(false);
	std::vector<SpriteRenderer*> renderables;
	renderables.reserve(sprites.size());
	for (const auto& sprite : sprites) {
		if (sprite && sprite->GetGameObject()) {
			renderables.push_back(sprite.get());
		}
	}

	std::sort(renderables.begin(), renderables.end(), [](const SpriteRenderer* a, const SpriteRenderer* b) {
		const int layerA = a->GetGameObject() ? a->GetGameObject()->GetLayer() : 0;
		const int layerB = b->GetGameObject() ? b->GetGameObject()->GetLayer() : 0;
		if (layerA != layerB) {
			return layerA < layerB;
		}
		if (a->m_layerOrder != b->m_layerOrder) {
			return a->m_layerOrder < b->m_layerOrder;
		}
		const size_t indexA = a->GetComponentIndex();
		const size_t indexB = b->GetComponentIndex();
		if (indexA != indexB) {
			return indexA < indexB;
		}
		return a->GetInstanceID() < b->GetInstanceID();
		});

	for (const auto* sprite : renderables) {
		sprite->Render(renderer);
	}
}

void SpriteRenderer::Render(Renderer& renderer) const {
	if (!m_texture || !m_texture->IsValid()) {
		return;
	}
	auto* transform = GetTransform();
	if (!transform) {
		return;
	}

	const Vector2i frameSize = GetResolvedFrameSize();
	const int maxFrames = GetMaxFrames(frameSize);
	if (maxFrames == 0) {
		return;
	}

	const int frameIndex = std::clamp(m_frameIndex, 0, maxFrames - 1);
	const Vector2i textureSize = m_texture->GetSize();
	const int columns = textureSize.x / frameSize.x;
	const int column = frameIndex % columns;
	const int row = frameIndex / columns;

	const float srcX = static_cast<float>(column * frameSize.x);
	const float srcY = static_cast<float>(row * frameSize.y);
	const float srcW = static_cast<float>(frameSize.x);
	const float srcH = static_cast<float>(frameSize.y);

	const Vector2f position = transform->GetWorldPosition();
	const Vector2f scale = transform->GetWorldScale();
	const float dstW = srcW * scale.x;
	const float dstH = srcH * scale.y;

	renderer.DrawTexture(*m_texture, Vector2f(srcX, srcY), Vector2f(srcW, srcH), position, Vector2f(dstW, dstH));
}

std::shared_ptr<Component> SpriteRenderer::Clone() const {
	auto clone = std::make_shared<SpriteRenderer>();
	clone->m_texture = m_texture;
	clone->m_frameSize = m_frameSize;
	clone->m_frameIndex = m_frameIndex;
	clone->m_layerOrder = m_layerOrder;
	return clone;
}

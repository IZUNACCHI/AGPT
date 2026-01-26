#include "TextRenderer.h"
#include "BitmapFont.h"
#include "Object.h"
#include "GameObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Texture.h"
#include <algorithm>
#include <cmath>
#include <vector>

static Vector2f RotateCCW(const Vector2f& v, float degrees) {
	const float rad = degrees * (Math::Constants<float>::Pi / 180.0f);
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	return Vector2f(v.x * c - v.y * s, v.x * s + v.y * c);
}

TextRenderer::TextRenderer()
	: Component("TextRenderer") {
}

void TextRenderer::RenderAll(Renderer& renderer) {
	auto texts = Object::FindObjectsByType<TextRenderer>(false);

	std::vector<TextRenderer*> renderables;
	renderables.reserve(texts.size());

	for (const auto& t : texts) {
		if (!t) continue;
		auto* go = t->GetGameObject();
		if (!go || !go->IsActiveInHierarchy()) continue;
		renderables.push_back(t.get());
	}

	std::sort(renderables.begin(), renderables.end(),
		[](const TextRenderer* a, const TextRenderer* b) {
			const int layerA = a->GetGameObject() ? a->GetGameObject()->GetLayer() : 0;
			const int layerB = b->GetGameObject() ? b->GetGameObject()->GetLayer() : 0;
			if (layerA != layerB) return layerA < layerB;

			if (a->m_layerOrder != b->m_layerOrder) return a->m_layerOrder < b->m_layerOrder;

			const size_t indexA = a->GetComponentIndex();
			const size_t indexB = b->GetComponentIndex();
			if (indexA != indexB) return indexA < indexB;

			return a->GetInstanceID() < b->GetInstanceID();
		}
	);

	for (auto* t : renderables) {
		t->Render(renderer);
	}
}

void TextRenderer::Render(Renderer& renderer) const {
	if (!m_font) return;

	auto* tr = GetTransform();
	if (!tr) return;

	Texture* tex = m_font->GetTexture();
	if (!tex || !tex->IsValid()) return;

	const Vector2f worldOrigin =

#include "TextRenderer.h"
#include "BitmapFont.h"
#include "Object.h"
#include "GameObject.h"
#include "Transform.h"
#include "Renderer.h"
#include "Texture.h"
#include <algorithm>
#include <cmath>

TextRenderer::TextRenderer()
	: RenderableComponent("TextRenderer") {
}

Vector2f TextRenderer::RotateDeg(const Vector2f& v, float deg) {
	const float r = deg * Math::Constants<float>::Deg2Rad;
	const float c = std::cos(r);
	const float s = std::sin(r);
	return Vector2f(v.x * c - v.y * s, v.x * s + v.y * c);
}

void TextRenderer::RenderAll(Renderer& renderer) {
	auto texts = Object::FindObjectsByType<TextRenderer>(false);

	std::vector<TextRenderer*> renderables;
	renderables.reserve(texts.size());

	for (const auto& t : texts) {
		if (!t) continue;
		if (!t->IsVisible()) continue;
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
	Texture* tex = m_font->GetTexture();
	if (!tex || !tex->IsValid()) return;

	auto* tr = GetTransform();
	if (!tr) return;

	const Vector2f anchorWorld = tr->GetWorldPosition();
	const float angleDeg = tr->GetWorldRotation();

	Vector2f s = tr->GetWorldScale();
	s.x *= m_extraScale;
	s.y *= m_extraScale;

	const float signX = (s.x < 0.0f) ? -1.0f : 1.0f;
	const float signY = (s.y < 0.0f) ? -1.0f : 1.0f;
	const Vector2f absScale(std::abs(s.x), std::abs(s.y));

	// Fast path: no rotation and no mirroring
	if (std::abs(angleDeg) < 0.0001f && signX > 0.0f && signY > 0.0f) {
		Vector2f start = anchorWorld;
		if (m_anchor == TextAnchor::Center) {
			const Vector2f size = m_font->MeasureText(m_text, absScale);
			start = anchorWorld + Vector2f(-size.x * 0.5f, +size.y * 0.5f);
		}
		m_font->Draw(renderer, m_text, start, absScale);
		return;
	}

	FlipMode flip = FlipMode::None;
	const bool flipX = signX < 0.0f;
	const bool flipY = signY < 0.0f;
	if (flipX && flipY) {
		flip = FlipMode::Both;
	}
	else if (flipX) {
		flip = FlipMode::Horizontal;
	}
	else if (flipY) {
		flip = FlipMode::Vertical;
	}

	const Vector2f blockSize = m_font->MeasureText(m_text, absScale);

	// local TOP-LEFT relative to anchor
	Vector2f originTL(0.0f, 0.0f);
	if (m_anchor == TextAnchor::Center) {
		originTL = Vector2f(-blockSize.x * 0.5f, +blockSize.y * 0.5f);
	}

	const Vector2i glyph = m_font->GetGlyphSize();
	const Vector2i sp = m_font->GetSpacing();

	const float advX = (float)(glyph.x + sp.x) * absScale.x;
	const float advY = (float)(glyph.y + sp.y) * absScale.y;

	const float glyphW = (float)glyph.x * absScale.x;
	const float glyphH = (float)glyph.y * absScale.y;

	Vector2f penTL = originTL;

	for (char ch : m_text) {
		if (ch == '\n') {
			penTL.x = originTL.x;
			penTL.y -= advY;
			continue;
		}

		Vector2f srcPos, srcSize;
		m_font->GetGlyphSourceRect(ch, srcPos, srcSize);

		// local center of this glyph (note: penTL is a TOP-LEFT in +Y up coords)
		Vector2f localCenter(
			penTL.x + glyphW * 0.5f,
			penTL.y - glyphH * 0.5f
		);

		// mirror positions if negative scale
		localCenter.x *= signX;
		localCenter.y *= signY;

		// rotate around anchor
		const Vector2f rotated = RotateDeg(localCenter, angleDeg);
		const Vector2f worldCenter = anchorWorld + rotated;

		// center -> world TOP-LEFT
		const Vector2f worldTopLeft = worldCenter + Vector2f(-glyphW * 0.5f, +glyphH * 0.5f);

		renderer.DrawTextureRotated(
			*tex,
			srcPos,
			srcSize,
			worldTopLeft,
			Vector2f(glyphW, glyphH),
			angleDeg,
			Vector2f(glyphW * 0.5f, glyphH * 0.5f),
			flip
		);

		penTL.x += advX;
	}
}

std::shared_ptr<Component> TextRenderer::Clone() const {
	auto clone = std::make_shared<TextRenderer>();
	clone->m_font = m_font;
	clone->m_text = m_text;
	clone->m_anchor = m_anchor;
	clone->m_extraScale = m_extraScale;
	clone->m_layerOrder = m_layerOrder;
	return clone;
}

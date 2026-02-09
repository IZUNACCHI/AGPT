#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include <GameEngine/GameEngine.h>
#include "XenonAssetKeys.h"

// Moves the owning GameObject at a constant speed in a WORLD direction.
// Rotation does not affect the movement direction.
class ParallaxMover2D : public MonoBehaviour {
public:
	ParallaxMover2D() : MonoBehaviour() {
		SetComponentName("ParallaxMover2D");
	}

	void Awake() override {
		t = GetTransform();
		CacheBoundsSources();
	}

	// Movement is in WORLD space; rotation does not affect the movement direction.
	void SetDirection(const Vector2f& dir) {
		m_direction = dir;
		if (m_direction.LengthSquared() > 0.0f) {
			m_direction = m_direction.Normalized();
		}
	}
	void SetSpeed(float pixelsPerSecond) { m_speed = pixelsPerSecond; }

	// Wrapping: when fully outside a side, teleport to the opposite side *offscreen*.
	void SetWrapEnabled(bool enabled) { m_wrapEnabled = enabled; }
	void SetWrapPadding(float pixels) { m_wrapPadding = std::max(0.0f, pixels); }
	void SetWrapSides(Viewport::Side sides) { m_wrapSides = sides; }

	void Update() override {
		if (!t) return;
		if (m_speed == 0.0f) return;
		if (m_direction.LengthSquared() <= 0.0f) return;

		t->SetPosition(t->GetWorldPosition() + m_direction * (m_speed * Time::DeltaTime()));
		WrapIfNeeded();
	}

private:
	void CacheBoundsSources() {
		m_sprites.clear();
		GameObject* go = GetGameObject();
		if (!go) return;

		auto sprites = go->GetComponentsInChildren<SpriteRenderer>();
		m_sprites.reserve(sprites.size());
		for (auto& s : sprites) {
			if (s) m_sprites.push_back(s.get());
		}
	}

	static void ExpandAABB(const Vector2f& mn, const Vector2f& mx, Vector2f& inoutMin, Vector2f& inoutMax, bool& inoutHas) {
		if (!inoutHas) {
			inoutMin = mn;
			inoutMax = mx;
			inoutHas = true;
			return;
		}
		inoutMin.x = std::min(inoutMin.x, mn.x);
		inoutMin.y = std::min(inoutMin.y, mn.y);
		inoutMax.x = std::max(inoutMax.x, mx.x);
		inoutMax.y = std::max(inoutMax.y, mx.y);
	}

	static bool SpriteWorldAABB(SpriteRenderer* sr, Vector2f& outMin, Vector2f& outMax) {
		if (!sr) return false;
		Transform* tr = sr->GetTransform();
		if (!tr) return false;

		const Vector2i fs = sr->GetResolvedFrameSize();
		if (fs.x <= 0 || fs.y <= 0) return false;

		const Vector2f ws = tr->GetWorldScale();
		const float w = fs.x * std::abs(ws.x);
		const float h = fs.y * std::abs(ws.y);

		const float hx = 0.5f * w;
		const float hy = 0.5f * h;

		const float rad = tr->GetWorldRotation() * (3.14159265358979323846f / 180.0f);
		const float c = std::cos(rad);
		const float s = std::sin(rad);

		// AABB extents of a rotated rect (sprite is centered)
		const float ax = std::abs(c) * hx + std::abs(s) * hy;
		const float ay = std::abs(s) * hx + std::abs(c) * hy;

		const Vector2f p = tr->GetWorldPosition();
		outMin = Vector2f(p.x - ax, p.y - ay);
		outMax = Vector2f(p.x + ax, p.y + ay);
		return true;
	}

	bool TryGetWorldAABB(Vector2f& outMin, Vector2f& outMax) {
		bool has = false;

		// Prefer sprites (works for block mosaics made of child SpriteRenderers).
		if (m_sprites.empty()) CacheBoundsSources();
		for (auto* sr : m_sprites) {
			Vector2f mn, mx;
			if (SpriteWorldAABB(sr, mn, mx)) {
				ExpandAABB(mn, mx, outMin, outMax, has);
			}
		}
		if (has) return true;

		// Fallback: BoxCollider2D on this GameObject (if any).
		if (auto col = GetComponent<Collider2D>().get()) {
			if (auto box = dynamic_cast<BoxCollider2D*>(col)) {
				const Vector2f size = box->GetSize();
				const Vector2f ws = t ? t->GetWorldScale() : Vector2f(1.0f, 1.0f);

				const float w = size.x * std::abs(ws.x);
				const float h = size.y * std::abs(ws.y);

				const float hx = 0.5f * w;
				const float hy = 0.5f * h;

				const float rad = (t ? t->GetWorldRotation() : 0.0f) * (3.14159265358979323846f / 180.0f);
				const float c = std::cos(rad);
				const float s = std::sin(rad);

				const float ax = std::abs(c) * hx + std::abs(s) * hy;
				const float ay = std::abs(s) * hx + std::abs(c) * hy;

				const Vector2f p = t ? t->GetWorldPosition() : Vector2f::Zero();
				outMin = Vector2f(p.x - ax, p.y - ay);
				outMax = Vector2f(p.x + ax, p.y + ay);
				return true;
			}
		}

		return false;
	}

	void WrapIfNeeded() {
		if (!m_wrapEnabled) return;
		if (!t) return;

		Renderer* renderer = GetRenderer();
		if (!renderer) return;

		const Vector2i vr = renderer->GetVirtualResolution();
		if (vr.x <= 0 || vr.y <= 0) return;

		Vector2f mn, mx;
		if (!TryGetWorldAABB(mn, mx)) return;

		const Viewport::Bounds b = Viewport::GetWorldBounds(vr);
		const float pad = m_wrapPadding;

		const float rectLeft = mn.x;
		const float rectRight = mx.x;
		const float rectBottom = mn.y;
		const float rectTop = mx.y;

		Vector2f pos = t->GetWorldPosition();
		Vector2f newPos = pos;

		// Wrap based on WORLD movement direction:
		//  - moving left  => when fully past LEFT, jump to just beyond RIGHT
		//  - moving right => when fully past RIGHT, jump to just beyond LEFT
		//  - moving down  => when fully past BOTTOM, jump to just beyond TOP
		//  - moving up    => when fully past TOP, jump to just beyond BOTTOM
		// Diagonal movement can wrap on both axes.

		// Horizontal wrap (left <-> right)
		if (Viewport::Any(m_wrapSides & Viewport::Side::Left)) {
			if (m_direction.x < 0.0f && rectRight < (b.left - pad)) {
				// Move so LEFT edge is just beyond the RIGHT side (offscreen).
				const float dx = (b.right + pad) - rectLeft;
				newPos.x += dx;
			}
		}
		if (Viewport::Any(m_wrapSides & Viewport::Side::Right)) {
			if (m_direction.x > 0.0f && rectLeft > (b.right + pad)) {
				// Move so RIGHT edge is just beyond the LEFT side (offscreen).
				const float dx = (b.left - pad) - rectRight;
				newPos.x += dx;
			}
		}

		// Vertical wrap (bottom <-> top)
		if (Viewport::Any(m_wrapSides & Viewport::Side::Bottom)) {
			if (m_direction.y < 0.0f && rectTop < (b.bottom - pad)) {
				const float dy = (b.top + pad) - rectBottom;
				newPos.y += dy;
			}
		}
		if (Viewport::Any(m_wrapSides & Viewport::Side::Top)) {
			if (m_direction.y > 0.0f && rectBottom > (b.top + pad)) {
				const float dy = (b.bottom - pad) - rectTop;
				newPos.y += dy;
			}
		}

		if (newPos.x != pos.x || newPos.y != pos.y) {
			t->SetPosition(newPos);
		}
	}

	Vector2f m_direction = Vector2f(-1.0f, 0.0f);
	float m_speed = 20.0f;
	Transform* t = nullptr;

	bool m_wrapEnabled = true;
	float m_wrapPadding = 8.0f;
	// Default: wrap on all sides. You can restrict this per object via SetWrapSides(...).
	Viewport::Side m_wrapSides = Viewport::Side::All;

	std::vector<SpriteRenderer*> m_sprites;
};

// Builds a rectangle out of tiles from Blocks.bmp (16 columns).
// Parameters are specified in FRAME coordinates (1-based) to avoid pixel math.
// Because SpriteRenderer renders a single frame, we create a mosaic of many tiles.
inline GameObject* CreateBlocksRect(Scene* scene,
	const std::string& name,
	int row1Based,
	int colStart1Based,
	int colEnd1Based,
	int rowsTall,
	const Vector2f& worldPosition,
	int layerOrder = -9)
{
	if (!scene) return nullptr;

	const int tilesPerRow = 16;
	const int tileW = 512 / tilesPerRow; // 32
	const Vector2i tileSize(tileW, tileW);

	Texture* tex = LoadTexture(XenonAssetKeys::Files::BlocksBmp, Vector3i(255, 0, 255));
	if (!tex) return nullptr;

	auto parent = scene->CreateGameObject<GameObject>(name).get();
	parent->GetTransform()->SetPosition(worldPosition);

	const int row0 = std::max(0, row1Based - 1);
	const int c0 = std::max(0, colStart1Based - 1);
	const int c1 = std::max(c0, colEnd1Based - 1);

	int tileIndex = 0;
	for (int ry = 0; ry < rowsTall; ++ry) {
		for (int cx = c0; cx <= c1; ++cx) {
			const int frameIndex = (row0 + ry) * tilesPerRow + cx;

			auto tile = scene->CreateGameObject<GameObject>(name + "_t" + std::to_string(tileIndex++));
			tile->GetTransform()->SetParent(parent->GetTransform());
			// Layout: build rightwards and downward in world space (negative Y for "down")
			tile->GetTransform()->SetPosition(Vector2f(
				(float)((cx - c0) * tileSize.x),
				(float)(-(ry) * tileSize.y)
			));

			auto sr = tile->AddComponent<SpriteRenderer>();
			sr->SetTexture(tex);
			sr->SetFrameSize(tileSize);
			sr->SetFrameIndex(frameIndex);
			sr->SetLayerOrder(layerOrder);
		}
	}

	return parent;
}

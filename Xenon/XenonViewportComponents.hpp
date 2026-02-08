#pragma once

#include <GameEngine/GameEngine.h>

#include <algorithm>
#include <cmath>

// Xenon-local viewport helper components.
// Kept in Xenon so you can tweak behavior per evaluation branch (horizontal/vertical scroller)
// without touching the engine.

// -----------------------------------------------------------------------------
// XenonClampToViewport2D
// -----------------------------------------------------------------------------
class XenonClampToViewport2D : public MonoBehaviour {
public:
	XenonClampToViewport2D() : MonoBehaviour() {
		SetComponentName("XenonClampToViewport2D");
	}

	void SetClampX(bool v) { m_clampX = v; }
	void SetClampY(bool v) { m_clampY = v; }
	void SetZeroVelocityOnClamp(bool v) { m_zeroVelocityOnClamp = v; }

	void LateUpdate() override {
		if (!GetGameObject()) return;

		Renderer* renderer = GetRenderer();
		if (!renderer) return;

		const Vector2i vr = renderer->GetVirtualResolution();
		if (vr.x <= 0 || vr.y <= 0) return;

			Transform* t = GetTransform();
		if (!t) return;

		// Axis-aligned half-extents in world units.
		Vector2f half = Vector2f::Zero();

		if (auto sr = GetComponent<SpriteRenderer>()) {
			const Vector2i fs = sr->GetResolvedFrameSize();
			const Vector2f ws = t->GetWorldScale();
			half.x = 0.5f * (fs.x > 0 ? fs.x * std::abs(ws.x) : 0.0f);
			half.y = 0.5f * (fs.y > 0 ? fs.y * std::abs(ws.y) : 0.0f);
		} else if (auto col = GetComponent<Collider2D>()) {
			if (auto box = dynamic_cast<BoxCollider2D*>(col.get())) {
				const Vector2f s = box->GetSize();
				const Vector2f ws = t->GetWorldScale();
				half.x = 0.5f * s.x * std::abs(ws.x);
				half.y = 0.5f * s.y * std::abs(ws.y);
			}
		}

		if (half.x <= 0.0f || half.y <= 0.0f) return;

		const Viewport::Bounds b = Viewport::GetWorldBounds(vr);

		Vector2f pos = t->GetWorldPosition();
		Vector2f newPos = pos;

		const float minX = b.left + half.x;
		const float maxX = b.right - half.x;
		const float minY = b.bottom + half.y;
			// NOTE: Viewport::Bounds::top is the top edge in world space.

		bool clampedX = false;
		bool clampedY = false;

		if (m_clampX) {
			if (newPos.x < minX) { newPos.x = minX; clampedX = true; }
			if (newPos.x > maxX) { newPos.x = maxX; clampedX = true; }
		}
		if (m_clampY) {
			const float minYc = b.bottom + half.y;
			const float maxYc = b.top - half.y;
			if (newPos.y < minYc) { newPos.y = minYc; clampedY = true; }
			if (newPos.y > maxYc) { newPos.y = maxYc; clampedY = true; }
		}

		if (newPos.x != pos.x || newPos.y != pos.y) {
			t->SetPosition(newPos);

			if (m_zeroVelocityOnClamp) {
				if (auto rb = GetComponent<Rigidbody2D>()) {
					Vector2f v = rb->GetLinearVelocity();
					if (clampedX) v.x = 0.0f;
					if (clampedY) v.y = 0.0f;
					rb->SetLinearVelocity(v);
				}
			}
		}
	}

private:
	bool m_clampX = true;
	bool m_clampY = true;
	bool m_zeroVelocityOnClamp = true;
};


// -----------------------------------------------------------------------------
// XenonDespawnOffscreen2D
// -----------------------------------------------------------------------------
class XenonDespawnOffscreen2D : public MonoBehaviour {
public:
	XenonDespawnOffscreen2D() : MonoBehaviour() {
		SetComponentName("XenonDespawnOffscreen2D");
	}

	void SetCheckInterval(float seconds) { m_interval = std::max(0.01f, seconds); }
	void SetRequireEntered(bool v) { m_requireEntered = v; }
	void SetSides(Viewport::Side sides) { m_sides = sides; }

	void Start() override {
		InvokeRepeating([this]() { Check(); }, m_interval, m_interval, MonoBehaviour::InvokeTickPolicy::WhileBehaviourEnabled);
	}

private:
	Viewport::WorldRect GetWorldRect() {
		Viewport::WorldRect r{};
		if (!GetGameObject()) return r;

		Transform* t = GetTransform();
		if (!t) return r;

		Vector2f half = Vector2f::Zero();

		if (auto sr = GetComponent<SpriteRenderer>().get()) {
			const Vector2i fs = sr->GetResolvedFrameSize();
			const Vector2f ws = t->GetWorldScale();
			half.x = 0.5f * (fs.x > 0 ? fs.x * std::abs(ws.x) : 0.0f);
			half.y = 0.5f * (fs.y > 0 ? fs.y * std::abs(ws.y) : 0.0f);
		} else if (auto col = GetComponent<Collider2D>().get()) {
			if (auto box = dynamic_cast<BoxCollider2D*>(col)) {
				const Vector2f s = box->GetSize();
				const Vector2f ws = t->GetWorldScale();
				half.x = 0.5f * s.x * std::abs(ws.x);
				half.y = 0.5f * s.y * std::abs(ws.y);
			}
		}

		const Vector2f pos = t->GetWorldPosition();
		r.topLeft = Vector2f(pos.x - half.x, pos.y + half.y);
		r.size = Vector2f(half.x * 2.0f, half.y * 2.0f);
		return r;
	}

	void Check() {
		GameObject* go = GetGameObject();
		if (!go) return;

		Renderer* renderer = GetRenderer();
		if (!renderer) return;

		const Vector2i vr = renderer->GetVirtualResolution();
		if (vr.x <= 0 || vr.y <= 0) return;

		const Viewport::WorldRect rect = GetWorldRect();
		if (rect.size.x <= 0.0f || rect.size.y <= 0.0f) return;

		const bool fullyOutside = Viewport::IsRectOutside(rect, vr, m_sides);

		if (m_requireEntered && !m_entered) {
			if (!fullyOutside) m_entered = true;
			return;
		}

		if (fullyOutside) {
			Object::Destroy(go);
		}
	}

	float m_interval = 0.25f;
	bool m_requireEntered = true;
	bool m_entered = false;
	Viewport::Side m_sides = Viewport::Side::All;
};

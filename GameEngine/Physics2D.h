#pragma once

#include <set>
#include <unordered_set>
#include <box2d/box2d.h>
#include "Types.hpp"

class Rigidbody2D;
class Collider2D;
class Renderer;

/// Wraps a Box2D world and registered physics components.
class Physics2DWorld {
public:
	/// Creates an empty physics world wrapper.
	Physics2DWorld() = default;
	/// Destroys the physics world wrapper.
	~Physics2DWorld();

	/// Initializes the Box2D world with the given gravity.
	void Initialize(const Vector2f& gravity = Vector2f(0.0f, 0.0f));
	/// Resets the Box2D world and rebuilds registered objects.
	void Reset(const Vector2f& gravity = Vector2f(0.0f, 0.0f));
	/// Shuts down the Box2D world.
	void Shutdown();
	/// Steps the Box2D world simulation.
	void Step(float timeStep, int subStepCount = 1);

	/// Debug draw registered collider shapes.
	void DebugDraw(Renderer& renderer) const;

	/// Returns the Box2D world handle.
	b2WorldId GetWorldId() const { return m_worldId; }
	/// Returns true if the world is valid.
	bool IsValid() const { return b2World_IsValid(m_worldId); }

	/// Sets the world gravity.
	void SetGravity(const Vector2f& gravity);
	/// Returns the world gravity.
	Vector2f GetGravity() const;

	/// Registers a rigidbody with the world.
	void RegisterBody(Rigidbody2D* body);
	/// Unregisters a rigidbody from the world.
	void UnregisterBody(Rigidbody2D* body);
	/// Registers a collider with the world.
	void RegisterCollider(Collider2D* collider);
	/// Unregisters a collider from the world.
	void UnregisterCollider(Collider2D* collider);

	/// Removes any cached collision/trigger pairs involving this collider.
	///
	/// Why this exists:
	/// - Box2D 3 gives us Begin/End events, but "Stay" must be synthesized.
	/// - If a collider's shape is recreated/destroyed mid-contact, Box2D may not
	///   emit an End event for the old contact.
	///
	/// Calling this prevents phantom OnCollisionStay / OnTriggerStay.
	void ClearContactCacheFor(Collider2D* collider);

private:
	/// Canonical (order-independent) pair of colliders.
	/// We always store the smaller pointer in 'a' so (A,B) and (B,A) are the same key.
	struct ColliderPair {
		Collider2D* a = nullptr;
		Collider2D* b = nullptr;
	};

	static ColliderPair MakePair(Collider2D* a, Collider2D* b) {
		return (a < b) ? ColliderPair{ a, b } : ColliderPair{ b, a };
	}

	/// Ordering for std::set.
	struct ColliderPairLess {
		bool operator()(const ColliderPair& x, const ColliderPair& y) const noexcept {
			if (x.a != y.a) {
				return x.a < y.a;
			}
			return x.b < y.b;
		}
	};

	/// Stored Box2D world handle.
	b2WorldId m_worldId = b2_nullWorldId;
	/// Registered rigidbodies.
	std::unordered_set<Rigidbody2D*> m_registeredBodies;
	/// Registered colliders.
	std::unordered_set<Collider2D*> m_registeredColliders;

	/// Active non-trigger contacts (used to synthesize OnCollisionStay).
	std::set<ColliderPair, ColliderPairLess> m_activeCollisions;
	/// Active trigger overlaps (used to synthesize OnTriggerStay).
	std::set<ColliderPair, ColliderPairLess> m_activeTriggers;
};

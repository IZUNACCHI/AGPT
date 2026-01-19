#pragma once

#include <unordered_set>
#include <box2d/box2d.h>
#include "Types.hpp"

class Rigidbody2D;
class Collider2D;

class Physics2DWorld {
public:
	Physics2DWorld() = default;
	~Physics2DWorld();

	void Initialize(const Vector2f& gravity = Vector2f(0.0f, -9.81f));
	void Reset(const Vector2f& gravity = Vector2f(0.0f, -9.81f));
	void Shutdown();
	void Step(float timeStep, int subStepCount = 1);

	b2WorldId GetWorldId() const { return m_worldId; }
	bool IsValid() const { return b2World_IsValid(m_worldId); }

	void SetGravity(const Vector2f& gravity);
	Vector2f GetGravity() const;

	void RegisterBody(Rigidbody2D* body);
	void UnregisterBody(Rigidbody2D* body);
	void RegisterCollider(Collider2D* collider);
	void UnregisterCollider(Collider2D* collider);

private:
	b2WorldId m_worldId = b2_nullWorldId;
	std::unordered_set<Rigidbody2D*> m_registeredBodies;
	std::unordered_set<Collider2D*> m_registeredColliders;
};

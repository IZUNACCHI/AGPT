#include "Physics2D.h"
#include "Collider2D.h"
#include "Rigidbody2D.h"

Physics2DWorld::~Physics2DWorld() {
	Shutdown();
}

void Physics2DWorld::Initialize(const Vector2f& gravity) {
	if (IsValid()) {
		return;
	}

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = { gravity.x, gravity.y };
	m_worldId = b2CreateWorld(&worldDef);
}

void Physics2DWorld::Reset(const Vector2f& gravity) {
	auto bodies = m_registeredBodies;
	auto colliders = m_registeredColliders;

	Shutdown();
	Initialize(gravity);

	for (auto* body : bodies) {
		if (body) {
			body->RecreateBody();
		}
	}

	for (auto* collider : colliders) {
		if (collider) {
			RegisterCollider(collider);
			collider->RebuildShape();
		}
	}
}

void Physics2DWorld::Shutdown() {
	if (!IsValid()) {
		return;
	}

	m_registeredBodies.clear();
	m_registeredColliders.clear();
	b2DestroyWorld(m_worldId);
	m_worldId = b2_nullWorldId;
}

void Physics2DWorld::Step(float timeStep, int subStepCount) {
	if (!IsValid()) {
		return;
	}

	b2World_Step(m_worldId, timeStep, subStepCount);

	for (auto* body : m_registeredBodies) {
		if (body) {
			body->SyncTransformFromBody();
		}
	}
}

void Physics2DWorld::SetGravity(const Vector2f& gravity) {
	if (!IsValid()) {
		return;
	}

	b2World_SetGravity(m_worldId, { gravity.x, gravity.y });
}

Vector2f Physics2DWorld::GetGravity() const {
	if (!IsValid()) {
		return Vector2f::Zero();
	}

	b2Vec2 gravity = b2World_GetGravity(m_worldId);
	return Vector2f(gravity.x, gravity.y);
}

void Physics2DWorld::RegisterBody(Rigidbody2D* body) {
	if (!body) {
		return;
	}
	m_registeredBodies.insert(body);
}

void Physics2DWorld::UnregisterBody(Rigidbody2D* body) {
	if (!body) {
		return;
	}
	m_registeredBodies.erase(body);
}

void Physics2DWorld::RegisterCollider(Collider2D* collider) {
	if (!collider) {
		return;
	}
	m_registeredColliders.insert(collider);
}

void Physics2DWorld::UnregisterCollider(Collider2D* collider) {
	if (!collider) {
		return;
	}
	m_registeredColliders.erase(collider);
}

#include "Physics2D.h"
#include "Collider2D.h"
#include "GameObject.h"
#include "MonoBehaviour.h"
#include "Renderer.h"
#include "Rigidbody2D.h"
#include "Transform.h"

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

	m_activeCollisions.clear();
	m_activeTriggers.clear();
	m_registeredBodies.clear();
	m_registeredColliders.clear();
	b2DestroyWorld(m_worldId);
	m_worldId = b2_nullWorldId;
}

Collider2D* ResolveColliderFromShape(b2ShapeId shapeId) {
	if (!b2Shape_IsValid(shapeId)) {
		return nullptr;
	}
	return static_cast<Collider2D*>(b2Shape_GetUserData(shapeId));
}

using CollisionCallback = void (MonoBehaviour::*)(Collider2D* other);

void DispatchCollisionEvent(Collider2D* collider, Collider2D* other, CollisionCallback callback) {
	if (!collider) {
		return;
	}

	auto* gameObject = collider->GetGameObject();
	if (!gameObject) {
		return;
	}

	auto behaviours = gameObject->GetComponents<MonoBehaviour>();
	for (const auto& behaviour : behaviours) {
		if (behaviour && behaviour->IsActiveAndEnabled()) {
			((*behaviour).*callback)(other);
		}
	}
}

void Physics2DWorld::Step(float timeStep, int subStepCount) {
	if (!IsValid()) {
		return;
	}

	b2World_Step(m_worldId, timeStep, subStepCount);

	const b2ContactEvents contactEvents = b2World_GetContactEvents(m_worldId);
	for (int i = 0; i < contactEvents.beginCount; ++i) {
		const auto& event = contactEvents.beginEvents[i];
		Collider2D* colliderA = ResolveColliderFromShape(event.shapeIdA);
		Collider2D* colliderB = ResolveColliderFromShape(event.shapeIdB);
		DispatchCollisionEvent(colliderA, colliderB, &MonoBehaviour::OnCollisionEnter);
		DispatchCollisionEvent(colliderB, colliderA, &MonoBehaviour::OnCollisionEnter);

		if (colliderA && colliderB) {
			m_activeCollisions.insert(MakePair(colliderA, colliderB));
		}
	}

	for (int i = 0; i < contactEvents.endCount; ++i) {
		const auto& event = contactEvents.endEvents[i];
		Collider2D* colliderA = ResolveColliderFromShape(event.shapeIdA);
		Collider2D* colliderB = ResolveColliderFromShape(event.shapeIdB);
		DispatchCollisionEvent(colliderA, colliderB, &MonoBehaviour::OnCollisionExit);
		DispatchCollisionEvent(colliderB, colliderA, &MonoBehaviour::OnCollisionExit);

		if (colliderA && colliderB) {
			m_activeCollisions.erase(MakePair(colliderA, colliderB));
		}
	}

	const b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_worldId);
	for (int i = 0; i < sensorEvents.beginCount; ++i) {
		const auto& event = sensorEvents.beginEvents[i];
		Collider2D* sensor = ResolveColliderFromShape(event.sensorShapeId);
		Collider2D* visitor = ResolveColliderFromShape(event.visitorShapeId);
		DispatchCollisionEvent(sensor, visitor, &MonoBehaviour::OnTriggerEnter);
		DispatchCollisionEvent(visitor, sensor, &MonoBehaviour::OnTriggerEnter);

		if (sensor && visitor) {
			m_activeTriggers.insert(MakePair(sensor, visitor));
		}
	}

	for (int i = 0; i < sensorEvents.endCount; ++i) {
		const auto& event = sensorEvents.endEvents[i];
		Collider2D* sensor = ResolveColliderFromShape(event.sensorShapeId);
		Collider2D* visitor = ResolveColliderFromShape(event.visitorShapeId);
		DispatchCollisionEvent(sensor, visitor, &MonoBehaviour::OnTriggerExit);
		DispatchCollisionEvent(visitor, sensor, &MonoBehaviour::OnTriggerExit);

		if (sensor && visitor) {
			m_activeTriggers.erase(MakePair(sensor, visitor));
		}
	}

	// "Stay" is not provided by Box2D 3's event API, so we synthesize it.
	// Anything still present in our active sets after processing Begin/End is considered "staying".
	for (const auto& p : m_activeCollisions) {
		DispatchCollisionEvent(p.a, p.b, &MonoBehaviour::OnCollisionStay);
		DispatchCollisionEvent(p.b, p.a, &MonoBehaviour::OnCollisionStay);
	}

	for (const auto& p : m_activeTriggers) {
		DispatchCollisionEvent(p.a, p.b, &MonoBehaviour::OnTriggerStay);
		DispatchCollisionEvent(p.b, p.a, &MonoBehaviour::OnTriggerStay);
	}

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
	ClearContactCacheFor(collider);
	m_registeredColliders.erase(collider);
}

void Physics2DWorld::ClearContactCacheFor(Collider2D* collider) {
	if (!collider) {
		return;
	}

	for (auto it = m_activeCollisions.begin(); it != m_activeCollisions.end(); ) {
		if (it->a == collider || it->b == collider) {
			it = m_activeCollisions.erase(it);
		}
		else {
			++it;
		}
	}

	for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end(); ) {
		if (it->a == collider || it->b == collider) {
			it = m_activeTriggers.erase(it);
		}
		else {
			++it;
		}
	}
}

void Physics2DWorld::DebugDraw(Renderer& renderer) const {
	for (auto* collider : m_registeredColliders) {
		if (!collider) continue;

		auto* gameObject = collider->GetGameObject();
		if (!gameObject) continue;

		auto* transform = gameObject->GetTransform();
		if (!transform) continue;

		const Vector2f worldPosition = transform->GetWorldPosition() + collider->GetOffset();
		const Vector3 color = collider->IsTrigger() ? Vector3(200, 0, 0) : Vector3(0, 200, 0);

		if (auto* box = dynamic_cast<BoxCollider2D*>(collider)) {
			const Vector2f size = box->GetSize();
			const Vector2f topLeft = worldPosition + Vector2f(-size.x * 0.5f, +size.y * 0.5f);
			renderer.DrawRectOutline(topLeft, size, color);
		}
		else if (auto* circle = dynamic_cast<CircleCollider2D*>(collider)) {
			renderer.DrawCircleOutline(worldPosition, circle->GetRadius(), color, 32);
		}
	}
}

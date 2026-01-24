#include "Collider2D.h"

#include "EngineException.hpp"
#include "Physics2D.h"
#include "Rigidbody2D.h"
#include "SleeplessEngine.h"

void Collider2D::Initialize() {
	auto* gameObject = GetGameObject();
	if (!gameObject) {
		return;
	}

	// Collider2D is expected to be attached to a Rigidbody2D.
	auto rb = gameObject->GetComponent<Rigidbody2D>();
	if (!rb) {
		THROW_ENGINE_EXCEPTION("GameObject '") << gameObject->GetName()
			<< "' must have a Rigidbody2D before adding a Collider2D";
	}

	if (auto* physicsWorld = SleeplessEngine::GetInstance().GetPhysicsWorld()) {
		physicsWorld->RegisterCollider(this);
	}

	AttachToRigidbody(rb.get());
	RecreateShape();
}

void Collider2D::Shutdown() {
	if (auto* physicsWorld = SleeplessEngine::GetInstance().GetPhysicsWorld()) {
		physicsWorld->UnregisterCollider(this);
	}

	if (b2Shape_IsValid(m_shapeId)) {
		b2DestroyShape(m_shapeId, true);
		m_shapeId = b2_nullShapeId;
	}

	m_attachedBody = nullptr;
}

void Collider2D::SetTrigger(bool isTrigger) {
	if (m_isTrigger == isTrigger) {
		return;
	}
	m_isTrigger = isTrigger;
	RecreateShape();
}

void Collider2D::RebuildShape() {
	RecreateShape();
}

void Collider2D::SetOffset(const Vector2f& offset) {
	m_offset = offset;
	RecreateShape();
}

void Collider2D::SetDensity(float density) {
	m_density = density;
	RecreateShape();
}

void Collider2D::SetFriction(float friction) {
	m_friction = friction;
	RecreateShape();
}

void Collider2D::SetRestitution(float restitution) {
	m_restitution = restitution;
	RecreateShape();
}

void Collider2D::AttachToRigidbody(Rigidbody2D* body) {
	if (m_attachedBody == body) {
		return;
	}

	m_attachedBody = body;
	RecreateShape();
}

void Collider2D::DetachFromRigidbody(Rigidbody2D* body) {
	if (m_attachedBody != body) {
		return;
	}

	m_attachedBody = nullptr;

	// Without a rigidbody, this collider can't exist in Box2D.
	if (b2Shape_IsValid(m_shapeId)) {
		b2DestroyShape(m_shapeId, true);
		m_shapeId = b2_nullShapeId;
	}
}

void Collider2D::DestroyImmediateInternal() {
	Shutdown();
	Component::DestroyImmediateInternal();
}

void Collider2D::RecreateShape() {
	b2BodyId bodyId = ResolveBody();
	if (!b2Body_IsValid(bodyId)) {
		if (b2Shape_IsValid(m_shapeId)) {
			b2DestroyShape(m_shapeId, true);
			m_shapeId = b2_nullShapeId;
		}
		return;
	}

	if (b2Shape_IsValid(m_shapeId)) {
		b2DestroyShape(m_shapeId, true);
		m_shapeId = b2_nullShapeId;
	}

	b2ShapeDef shapeDef = BuildShapeDef();
	m_shapeId = CreateShape(bodyId, shapeDef);
}

b2ShapeDef Collider2D::BuildShapeDef() const {
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.userData = const_cast<Collider2D*>(this);
	shapeDef.density = m_density;
	shapeDef.material.friction = m_friction;
	shapeDef.material.restitution = m_restitution;

	shapeDef.isSensor = m_isTrigger;
	shapeDef.enableSensorEvents = m_shouldSensorEvent;
	shapeDef.enableContactEvents = !m_isTrigger;
	shapeDef.updateBodyMass = true;

	return shapeDef;
}

b2BodyId Collider2D::ResolveBody() const {
	if (!m_attachedBody) {
		return b2_nullBodyId;
	}

	b2BodyId bodyId = m_attachedBody->GetBodyId();
	if (!b2Body_IsValid(bodyId)) {
		return b2_nullBodyId;
	}

	return bodyId;
}

void BoxCollider2D::SetSize(const Vector2f& size) {
	m_size = size;
	RecreateShape();
}

std::shared_ptr<Component> BoxCollider2D::Clone() const {
	auto clone = std::make_shared<BoxCollider2D>();
	clone->m_offset = m_offset;
	clone->m_density = m_density;
	clone->m_friction = m_friction;
	clone->m_restitution = m_restitution;
	clone->m_isTrigger = m_isTrigger;
	clone->m_size = m_size;
	clone->SetComponentName(GetComponentName());
	return clone;
}

b2ShapeId BoxCollider2D::CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) {
	b2Polygon box = b2MakeOffsetBox(
		m_size.x * 0.5f,
		m_size.y * 0.5f,
		{ m_offset.x, m_offset.y },
		b2MakeRot(0.0f)
	);
	return b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

void CircleCollider2D::SetRadius(float radius) {
	m_radius = radius;
	RecreateShape();
}

std::shared_ptr<Component> CircleCollider2D::Clone() const {
	auto clone = std::make_shared<CircleCollider2D>();
	clone->m_offset = m_offset;
	clone->m_density = m_density;
	clone->m_friction = m_friction;
	clone->m_restitution = m_restitution;
	clone->m_isTrigger = m_isTrigger;
	clone->m_radius = m_radius;
	clone->SetComponentName(GetComponentName());
	return clone;
}

b2ShapeId CircleCollider2D::CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) {
	b2Circle circle{};
	circle.center = { m_offset.x, m_offset.y };
	circle.radius = m_radius;
	return b2CreateCircleShape(bodyId, &shapeDef, &circle);
}
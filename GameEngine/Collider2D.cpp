#include "Collider2D.h"
#include "Physics2D.h"
#include "Rigidbody2D.h"
#include "SleeplessEngine.h"
#include "Transform.h"

void Collider2D::Initialize() {
	if (auto* physicsWorld = SleeplessEngine::GetInstance().GetPhysicsWorld()) {
		physicsWorld->RegisterCollider(this);
	}

	auto rigidbody = GetGameObject() ? GetGameObject()->GetComponent<Rigidbody2D>() : nullptr;
	if (rigidbody) {
		AttachToRigidbody(rigidbody.get());
	}

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

	if (m_ownsBody && b2Body_IsValid(m_staticBodyId)) {
		b2DestroyBody(m_staticBodyId);
		m_staticBodyId = b2_nullBodyId;
	}
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
	if (m_ownsBody && b2Body_IsValid(m_staticBodyId)) {
		b2DestroyBody(m_staticBodyId);
		m_staticBodyId = b2_nullBodyId;
		m_ownsBody = false;
	}

	RecreateShape();
}

void Collider2D::DetachFromRigidbody(Rigidbody2D* body) {
	if (m_attachedBody != body) {
		return;
	}

	m_attachedBody = nullptr;
	RecreateShape();
}

void Collider2D::DestroyImmediateInternal() {
	Shutdown();
	Component::DestroyImmediateInternal();
}

void Collider2D::RecreateShape() {
	if (b2Shape_IsValid(m_shapeId)) {
		b2DestroyShape(m_shapeId, true);
		m_shapeId = b2_nullShapeId;
	}

	b2BodyId bodyId = ResolveBody();
	if (!b2Body_IsValid(bodyId)) {
		return;
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
	shapeDef.enableSensorEvents = m_isTrigger;
	shapeDef.enableContactEvents = !m_isTrigger;
	shapeDef.updateBodyMass = true;
	return shapeDef;
}

b2BodyId Collider2D::ResolveBody() {
	if (m_attachedBody && b2Body_IsValid(m_attachedBody->GetBodyId())) {
		m_ownsBody = false;
		return m_attachedBody->GetBodyId();
	}

	if (m_ownsBody && b2Body_IsValid(m_staticBodyId)) {
		return m_staticBodyId;
	}

	auto* physicsWorld = SleeplessEngine::GetInstance().GetPhysicsWorld();
	if (!physicsWorld || !physicsWorld->IsValid()) {
		return b2_nullBodyId;
	}

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.userData = this;

	auto* transform = GetGameObject()->GetTransform();
	Vector2f position = transform->GetWorldPosition();
	bodyDef.position = { position.x, position.y };
	bodyDef.rotation = b2MakeRot(transform->GetWorldRotation() * Math::Constants<float>::Deg2Rad);

	m_staticBodyId = b2CreateBody(physicsWorld->GetWorldId(), &bodyDef);
	m_ownsBody = true;
	return m_staticBodyId;
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
	return clone;
}

b2ShapeId BoxCollider2D::CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) {
	b2Polygon box = b2MakeOffsetBox(m_size.x * 0.5f, m_size.y * 0.5f, { m_offset.x, m_offset.y }, b2MakeRot(0.0f));
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
	return clone;
}

b2ShapeId CircleCollider2D::CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) {
	b2Circle circle{};
	circle.center = { m_offset.x, m_offset.y };
	circle.radius = m_radius;
	return b2CreateCircleShape(bodyId, &shapeDef, &circle);
}
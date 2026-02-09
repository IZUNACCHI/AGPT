#include "Rigidbody2D.h"
#include "Collider2D.h"
#include "Physics2D.h"
#include "SleeplessEngine.h"
#include "Transform.h"

namespace {
	b2Vec2 ToB2Vec(const Vector2f& vector) {
		return { vector.x, vector.y };
	}

	Vector2f FromB2Vec(const b2Vec2& vector) {
		return Vector2f(vector.x, vector.y);
	}

	Physics2DWorld* GetPhysicsWorld() {
		return SleeplessEngine::GetInstance().GetPhysicsWorld();
	}
}

void Rigidbody2D::Initialize() {
	CreateBody();
	AttachExistingColliders();
}

void Rigidbody2D::Shutdown() {
	DetachExistingColliders();
	DestroyBody();
}

void Rigidbody2D::RecreateBody() {
	DestroyBody();
	CreateBody();
	AttachExistingColliders();
}

void Rigidbody2D::SetBodyType(BodyType type) {
	m_bodyType = type;
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_SetType(m_bodyId, static_cast<b2BodyType>(m_bodyType));
	}
}

void Rigidbody2D::SetGravityScale(float scale) {
	m_gravityScale = scale;
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_SetGravityScale(m_bodyId, scale);
	}
}

void Rigidbody2D::SetLinearVelocity(const Vector2f& velocity) {
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_SetLinearVelocity(m_bodyId, ToB2Vec(velocity));
	}
}

Vector2f Rigidbody2D::GetLinearVelocity() const {
	if (!b2Body_IsValid(m_bodyId)) {
		return Vector2f::Zero();
	}
	return FromB2Vec(b2Body_GetLinearVelocity(m_bodyId));
}

void Rigidbody2D::SetAngularVelocity(float velocity) {
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_SetAngularVelocity(m_bodyId, velocity);
	}
}

float Rigidbody2D::GetAngularVelocity() const {
	if (!b2Body_IsValid(m_bodyId)) {
		return 0.0f;
	}
	return b2Body_GetAngularVelocity(m_bodyId);
}

void Rigidbody2D::ApplyForce(const Vector2f& force, const Vector2f& point, bool wake) {
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_ApplyForce(m_bodyId, ToB2Vec(force), ToB2Vec(point), wake);
	}
}

void Rigidbody2D::ApplyForceToCenter(const Vector2f& force, bool wake) {
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_ApplyForceToCenter(m_bodyId, ToB2Vec(force), wake);
	}
}

void Rigidbody2D::ApplyLinearImpulse(const Vector2f& impulse, const Vector2f& point, bool wake) {
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_ApplyLinearImpulse(m_bodyId, ToB2Vec(impulse), ToB2Vec(point), wake);
	}
}

void Rigidbody2D::ApplyLinearImpulseToCenter(const Vector2f& impulse, bool wake) {
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_ApplyLinearImpulseToCenter(m_bodyId, ToB2Vec(impulse), wake);
	}
}

void Rigidbody2D::SetPosition(const Vector2f& position) {
	if (!b2Body_IsValid(m_bodyId)) {
		return;
	}

	b2Rot rotation = b2Body_GetRotation(m_bodyId);
	b2Body_SetTransform(m_bodyId, ToB2Vec(position), rotation);
	// Keep the Transform in sync *without* writing back into Box2D again.
	GetGameObject()->GetTransform()->SetWorldPositionFromPhysics(position);
}

void Rigidbody2D::SetRotation(float rotationDegrees) {
	SetRotationRadians(rotationDegrees * Math::Constants<float>::Deg2Rad);
}

void Rigidbody2D::SetRotationRadians(float rotationRadians) {
	if (!b2Body_IsValid(m_bodyId)) {
		return;
	}

	b2MotionLocks locks = b2Body_GetMotionLocks(m_bodyId);
	const bool lockedAngular = locks.angularZ;
	if (lockedAngular) {
		locks.angularZ = false;
		b2Body_SetMotionLocks(m_bodyId, locks);
	}

	b2Vec2 position = b2Body_GetPosition(m_bodyId);
	b2Body_SetTransform(m_bodyId, position, b2MakeRot(rotationRadians));
	// Keep the Transform in sync *without* writing back into Box2D again.
	GetGameObject()->GetTransform()->SetWorldRotationFromPhysics(rotationRadians * Math::Constants<float>::Rad2Deg);

	if (lockedAngular) {
		locks.angularZ = true;
		b2Body_SetMotionLocks(m_bodyId, locks);
	}
}

void Rigidbody2D::SetLinearDamping(float damping) {
	m_linearDamping = damping;
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_SetLinearDamping(m_bodyId, damping);
	}
}

void Rigidbody2D::SetAngularDamping(float damping) {
	m_angularDamping = damping;
	if (b2Body_IsValid(m_bodyId)) {
		b2Body_SetAngularDamping(m_bodyId, damping);
	}
}

void Rigidbody2D::SetFixedRotation(bool fixedRotation) {
	m_fixedRotation = fixedRotation;
	if (b2Body_IsValid(m_bodyId)) {
		b2MotionLocks locks = b2Body_GetMotionLocks(m_bodyId);
		locks.angularZ = fixedRotation;
		b2Body_SetMotionLocks(m_bodyId, locks);
	}
}

void Rigidbody2D::SetIsBullet(bool isBullet)
{
	m_isBullet = isBullet;
}

void Rigidbody2D::SyncTransformFromBody() {
	if (!b2Body_IsValid(m_bodyId)) {
		return;
	}

	b2Vec2 position = b2Body_GetPosition(m_bodyId);
	b2Rot rotation = b2Body_GetRotation(m_bodyId);
	float angle = b2Rot_GetAngle(rotation);

	auto* transform = GetGameObject()->GetTransform();
	transform->SetWorldPositionFromPhysics(Vector2f(position.x, position.y));
	transform->SetWorldRotationFromPhysics(angle * Math::Constants<float>::Rad2Deg);
}

std::shared_ptr<Component> Rigidbody2D::Clone() const {
	auto clone = std::make_shared<Rigidbody2D>();
	clone->m_bodyType = m_bodyType;
	clone->m_gravityScale = m_gravityScale;
	clone->m_linearDamping = m_linearDamping;
	clone->m_angularDamping = m_angularDamping;
	clone->m_fixedRotation = m_fixedRotation;
	clone->m_allowSleep = m_allowSleep;
	clone->m_isBullet = m_isBullet;
	return clone;
}

void Rigidbody2D::CreateBody() {
	auto* physicsWorld = GetPhysicsWorld();
	if (!physicsWorld || !physicsWorld->IsValid()) {
		return;
	}

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = static_cast<b2BodyType>(m_bodyType);
	bodyDef.gravityScale = m_gravityScale;
	bodyDef.linearDamping = m_linearDamping;
	bodyDef.angularDamping = m_angularDamping;
	bodyDef.isBullet = m_isBullet;
	bodyDef.enableSleep = m_allowSleep;
	bodyDef.userData = this;

	auto* transform = GetGameObject()->GetTransform();
	Vector2f position = transform->GetWorldPosition();
	bodyDef.position = { position.x, position.y };
	bodyDef.rotation = b2MakeRot(transform->GetWorldRotation() * Math::Constants<float>::Deg2Rad);
	bodyDef.motionLocks.angularZ = m_fixedRotation;

	m_bodyId = b2CreateBody(physicsWorld->GetWorldId(), &bodyDef);
	physicsWorld->RegisterBody(this);
}

void Rigidbody2D::DestroyBody() {
	auto* physicsWorld = GetPhysicsWorld();
	if (physicsWorld) {
		physicsWorld->UnregisterBody(this);
	}

	if (b2Body_IsValid(m_bodyId)) {
		b2DestroyBody(m_bodyId);
		m_bodyId = b2_nullBodyId;
	}
}

void Rigidbody2D::AttachExistingColliders() {
	auto colliders = GetGameObject()->GetComponents<Collider2D>();
	for (const auto& collider : colliders) {
		if (collider) {
			collider->AttachToRigidbody(this);
		}
	}
}

void Rigidbody2D::DetachExistingColliders() {
	auto colliders = GetGameObject()->GetComponents<Collider2D>();
	for (const auto& collider : colliders) {
		if (collider) {
			collider->DetachFromRigidbody(this);
		}
	}
}

void Rigidbody2D::DestroyImmediateInternal() {
	Shutdown();
	Component::DestroyImmediateInternal();
}

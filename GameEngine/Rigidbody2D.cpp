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

void Rigidbody2D::OnCreate() {
	if (IsEffectivelyEnabled()) {
		CreateBody();
		AttachExistingColliders();
	}
}

void Rigidbody2D::OnEnable() {
	CreateBody();
	AttachExistingColliders();
}

void Rigidbody2D::OnDisable() {
	DetachExistingColliders();
	DestroyBody();
}

void Rigidbody2D::OnDestroy() {
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

@@ - 138, 78 + 150, 86 @@ void Rigidbody2D::SetAngularDamping(float damping) {

	void Rigidbody2D::SetFixedRotation(bool fixedRotation) {
		m_fixedRotation = fixedRotation;
		if (b2Body_IsValid(m_bodyId)) {
			b2MotionLocks locks = b2Body_GetMotionLocks(m_bodyId);
			locks.angularZ = fixedRotation;
			b2Body_SetMotionLocks(m_bodyId, locks);
		}
	}

	void Rigidbody2D::SyncTransformFromBody() {
		if (!b2Body_IsValid(m_bodyId)) {
			return;
		}

		b2Vec2 position = b2Body_GetPosition(m_bodyId);
		b2Rot rotation = b2Body_GetRotation(m_bodyId);
		float angle = b2Rot_GetAngle(rotation);

		auto* transform = GetGameObject()->GetTransform();
		transform->SetPosition(Vector2f(position.x, position.y));
		transform->SetRotationRadians(angle);
	}

	void Rigidbody2D::CreateBody() {
		if (b2Body_IsValid(m_bodyId)) {
			return;
		}

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
		if (!b2Body_IsValid(m_bodyId)) {
			return;
		}

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
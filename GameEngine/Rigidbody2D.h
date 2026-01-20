#pragma once

#include <box2d/box2d.h>
#include "Component.h"
#include "Types.hpp"

class Collider2D;

class Rigidbody2D : public Component {
public:
	enum class BodyType {
		Static = b2_staticBody,
		Kinematic = b2_kinematicBody,
		Dynamic = b2_dynamicBody
	};

	void OnCreate() override;
	void OnEnable() override;
	void OnDisable() override;
	void OnDestroy() override;
	void RecreateBody();

	b2BodyId GetBodyId() const { return m_bodyId; }

	BodyType GetBodyType() const { return m_bodyType; }
	void SetBodyType(BodyType type);

	void SetGravityScale(float scale);
	float GetGravityScale() const { return m_gravityScale; }

	void SetLinearVelocity(const Vector2f& velocity);
	Vector2f GetLinearVelocity() const;

	void SetAngularVelocity(float velocity);
	float GetAngularVelocity() const;

	void ApplyForce(const Vector2f& force, const Vector2f& point, bool wake = true);
	void ApplyForceToCenter(const Vector2f& force, bool wake = true);
	void ApplyLinearImpulse(const Vector2f& impulse, const Vector2f& point, bool wake = true);
	void ApplyLinearImpulseToCenter(const Vector2f& impulse, bool wake = true);

	void SetPosition(const Vector2f& position);
	void SetRotation(float rotationDegrees);
	void SetRotationRadians(float rotationRadians);
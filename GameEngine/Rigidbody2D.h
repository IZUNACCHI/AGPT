#pragma once

#include <box2d/box2d.h>
#include "Component.h"
#include "Types.hpp"

class Collider2D;

// Rigidbody component that drives physics simulation via Box2D
class Rigidbody2D : public Component {
public:
	// Supported rigidbody body types.
	enum class BodyType {
		Static = b2_staticBody,
		Kinematic = b2_kinematicBody,
		Dynamic = b2_dynamicBody
	};

	// Creates a rigidbody component with default settings
	Rigidbody2D() = default;
	// Releases rigidbody resources
	~Rigidbody2D() override = default;

	// Initializes the rigidbody with the physics world
	void Initialize();
	// Shuts down the rigidbody and releases physics resources
	void Shutdown();
	// Recreates the Box2D body with current settings
	void RecreateBody();

	// Returns the Box2D body handle
	b2BodyId GetBodyId() const { return m_bodyId; }

	// Returns the current body type
	BodyType GetBodyType() const { return m_bodyType; }
	// Sets the body type for this rigidbody
	void SetBodyType(BodyType type);

	// Sets the gravity scale applied to this rigidbody
	void SetGravityScale(float scale);
	// Returns the gravity scale applied to this rigidbody
	float GetGravityScale() const { return m_gravityScale; }

	// Sets the linear velocity
	void SetLinearVelocity(const Vector2f& velocity);
	// Returns the current linear velocity
	Vector2f GetLinearVelocity() const;

	// Sets the angular velocity in radians/sec
	void SetAngularVelocity(float velocity);
	// Returns the current angular velocity
	float GetAngularVelocity() const;

	// Applies a force at a world-space point
	void ApplyForce(const Vector2f& force, const Vector2f& point, bool wake = true);
	// Applies a force at the center of mass
	void ApplyForceToCenter(const Vector2f& force, bool wake = true);
	// Applies a linear impulse at a world-space point
	void ApplyLinearImpulse(const Vector2f& impulse, const Vector2f& point, bool wake = true);
	// Applies a linear impulse at the center of mass
	void ApplyLinearImpulseToCenter(const Vector2f& impulse, bool wake = true);

	// Sets the world position of the rigidbody
	void SetPosition(const Vector2f& position);
	// Sets the world rotation in degrees
	void SetRotation(float rotationDegrees);
	// Sets the world rotation in radians
	void SetRotationRadians(float rotationRadians);

	// Sets the linear damping coefficient
	void SetLinearDamping(float damping);
	// Sets the angular damping coefficient
	void SetAngularDamping(float damping);

	// Locks or unlocks rotation around the Z axis
	void SetFixedRotation(bool fixedRotation);

	// Sets whether the body can use continuous collision detection
	void SetIsBullet(bool isBullet);
	
	// Returns whether the body can use continuous collision detection
	bool IsBullet() const { return m_isBullet; }



	/// Syncs the Transform from the current Box2D body state.
	void SyncTransformFromBody();
	/// Clones the rigidbody settings.
	std::shared_ptr<Component> Clone() const override;

private:
	/// Creates the Box2D body for this rigidbody.
	void CreateBody();
	/// Destroys the Box2D body for this rigidbody.
	void DestroyBody();
	/// Attaches any colliders on the same GameObject.
	void AttachExistingColliders();
	/// Detaches any colliders on the same GameObject.
	void DetachExistingColliders();
	/// Tears down rigidbody resources when destroyed.
	void DestroyImmediateInternal() override;

	/// Stored Box2D body handle.
	b2BodyId m_bodyId = b2_nullBodyId;
	/// Current body type.
	BodyType m_bodyType = BodyType::Dynamic;
	/// Gravity scale applied to the body.
	float m_gravityScale = 1.0f;
	/// Linear damping applied to the body.
	float m_linearDamping = 0.0f;
	/// Angular damping applied to the body.
	float m_angularDamping = 0.0f;
	/// Whether rotation is locked on the Z axis.
	bool m_fixedRotation = false;
	/// Whether the body is allowed to sleep.
	bool m_allowSleep = true;
	/// Whether the body uses continuous collision.
	bool m_isBullet = false;
};
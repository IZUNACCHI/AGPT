#pragma once

#include <box2d/box2d.h>
#include <memory>
#include "Component.h"
#include "Types.hpp"

class Rigidbody2D;

// Base collider component that owns a Box2D shape
class Collider2D : public Component {
public:
	// Creates a collider component with default settings
	Collider2D() = default;
	// Releases collider resources.
	~Collider2D() override = default;

	// Initializes the collider and creates its physics shape
	void Initialize();
	// Shuts down the collider and releases physics resources
	void Shutdown();
	// Rebuilds the Box2D shape when the physics world is reset
	void RebuildShape();

	// Returns whether the collider behaves as a trigger
	bool IsTrigger() const { return m_isTrigger; }
	// Sets whether the collider behaves as a trigger
	void SetTrigger(bool isTrigger);


	// Returns whether sensor events should be generated for this collider
	bool ShouldSensorEvent() const { return m_shouldSensorEvent; }
	void SetShouldSensorEvent(bool shouldEvent) { m_shouldSensorEvent = shouldEvent; }

	// Sets the local offset of the collider shape
	void SetOffset(const Vector2f& offset);
	// Returns the local offset of the collider shape
	Vector2f GetOffset() const { return m_offset; }

	// Sets the collider density used for body mass
	void SetDensity(float density);
	// Sets the collider friction.
	void SetFriction(float friction);
	// Sets the collider restitution (bounciness)
	void SetRestitution(float restitution);

	// Attaches the collider to a rigidbody.
	void AttachToRigidbody(Rigidbody2D* body);
	// Detaches the collider from a rigidbody (collider becomes inactive until reattached)
	void DetachFromRigidbody(Rigidbody2D* body);

	// Returns the Box2D shape handle for this collider
	b2ShapeId GetShapeId() const { return m_shapeId; }

protected:
	// Creates the Box2D shape for this collider type.
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) = 0;
	// Tears down collider resources when destroyed.
	void DestroyImmediateInternal() override;

	// Recreates the Box2D shape on the current body.
	void RecreateShape();
	// Builds the Box2D shape definition from collider settings.
	b2ShapeDef BuildShapeDef() const;
	// Returns the body to attach shapes to (requires Rigidbody2D).
	b2BodyId ResolveBody() const;

	b2ShapeId m_shapeId = b2_nullShapeId; // Box2D shape handle
	Rigidbody2D* m_attachedBody = nullptr; // Attached rigidbody (if any)

	Vector2f m_offset = Vector2f::Zero(); // Local shape offset
	float m_density = 1.0f; // Shape density for mass calculation
	float m_friction = 0.3f; // Shape friction
	float m_restitution = 0.0f; // Shape restitution (bounciness)
	bool m_isTrigger = false; // Whether the collider is a trigger
	bool m_shouldSensorEvent = true; // Whether sensor events should be generated
};

/// Box-shaped collider component.
class BoxCollider2D : public Collider2D {
public:
	/// Sets the local size of the box shape.
	void SetSize(const Vector2f& size);
	/// Returns the local size of the box shape.
	Vector2f GetSize() const { return m_size; }
	/// Clones the collider settings.
	std::shared_ptr<Component> Clone() const override;

protected:
	/// Creates the Box2D polygon shape for the box.
	b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override;

private:
	Vector2f m_size = Vector2f(1.0f, 1.0f);
};

/// Circle-shaped collider component.
class CircleCollider2D : public Collider2D {
public:
	/// Sets the radius of the circle shape.
	void SetRadius(float radius);
	/// Returns the radius of the circle shape.
	float GetRadius() const { return m_radius; }
	/// Clones the collider settings.
	std::shared_ptr<Component> Clone() const override;

protected:
	/// Creates the Box2D circle shape for the collider.
	b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override;

private:
	float m_radius = 0.5f;
};

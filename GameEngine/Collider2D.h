#pragma once

#include <box2d/box2d.h>
#include "Component.h"
#include "Renderer.h"
#include "Types.hpp"

class Rigidbody2D;

class Collider2D : public Component {
public:
	// Register with physics world and build initial shape.
	void OnCreate() override;
	// Unregister and release Box2D resources.
	void OnDestroy() override;
	// Rebuild the collider shape when the physics world is reset.
	void RebuildShape();
	// Debug-render the collider shape if enabled.
	void Draw() ;

	bool IsTrigger() const { return m_isTrigger; }
	void SetTrigger(bool isTrigger);

	void SetOffset(const Vector2f& offset);
	Vector2f GetOffset() const { return m_offset; }

	void SetDensity(float density);
	void SetFriction(float friction);
	void SetRestitution(float restitution);

	void AttachToRigidbody(Rigidbody2D* body);
	void DetachFromRigidbody(Rigidbody2D* body);

	b2ShapeId GetShapeId() const { return m_shapeId; }
	// Toggle debug visualization.
	void SetDebugDraw(bool enabled) { m_debugDraw = enabled; }
	bool IsDebugDrawEnabled() const { return m_debugDraw; }

protected:
	// Create the Box2D shape for this collider type.
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) = 0;
	// Render a simple debug outline for this collider type.
	virtual void DrawDebugShape(Renderer& renderer) = 0;
	void RecreateShape();
	b2ShapeDef BuildShapeDef() const;
	b2BodyId ResolveBody();

	b2ShapeId m_shapeId = b2_nullShapeId;
	b2BodyId m_staticBodyId = b2_nullBodyId;
	Rigidbody2D* m_attachedBody = nullptr;
	bool m_ownsBody = false;

	Vector2f m_offset = Vector2f::Zero();
	float m_density = 1.0f;
	float m_friction = 0.3f;
	float m_restitution = 0.0f;
	bool m_isTrigger = false;
	bool m_debugDraw = true;
};

class BoxCollider2D : public Collider2D {
public:
	void SetSize(const Vector2f& size);
	Vector2f GetSize() const { return m_size; }

protected:
	b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override;
	void DrawDebugShape(Renderer& renderer) override;

private:
	Vector2f m_size = Vector2f(1.0f, 1.0f);
};

class CircleCollider2D : public Collider2D {
public:
	void SetRadius(float radius);
	float GetRadius() const { return m_radius; }

protected:
	b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef) override;
	void DrawDebugShape(Renderer& renderer) override;

private:
	float m_radius = 0.5f;
};
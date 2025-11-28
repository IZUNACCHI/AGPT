#pragma once
#include "Vec2.h"
#include "Component.h"

class Transform : public Component {
public:
	// Local properties
	Vec2 localPosition = { 0.0f, 0.0f };
	float localRotation = 0.0f; // degrees
	Vec2 localScale = { 1.0f, 1.0f };

	// --- Local Convenience Mutators ---
	void SetLocalPosition(float x, float y);
	void MoveLocal(float x, float y);
	void SetLocalRotation(float degrees);
	void RotateLocal(float degrees);
	void SetLocalScale(float x, float y);
	void ScaleLocal(float x, float y);

	// --- World Getters ---
	Vec2 GetWorldPosition() const;
	float GetWorldRotation() const;
	Vec2 GetWorldScale() const;

	// --- World Setters (Convert to local) ---
	void SetWorldPosition(const Vec2& position);
	void SetWorldRotation(float degrees);

	// --- New World Convenience Mutators (Setters that use relative movement) ---
	void MoveWorld(const Vec2& deltaWorld);
	void RotateWorld(float degreesDelta);

private:
	Transform* GetParentTransform() const;
};

static constexpr float PI = 3.14159265358979323846f;
static constexpr float DEG_TO_RAD = PI / 180.0f;
// static constexpr float RAD_TO_DEG = 180.0f / PI;
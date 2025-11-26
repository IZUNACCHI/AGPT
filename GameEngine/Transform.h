// Transform.h
#pragma once
#include "Component.h"
#include <cmath>

class Transform : public Component {
public:
	// Local properties
	float localPositionX = 0.0f;
	float localPositionY = 0.0f;
	float localRotation = 0.0f; // degrees
	float localScaleX = 1.0f;
	float localScaleY = 1.0f;

	// World getters
	float GetWorldPositionX() const;
	float GetWorldPositionY() const;
	float GetWorldRotation() const;
	float GetWorldScaleX() const;
	float GetWorldScaleY() const;

	// World setters (convert to local)
	void SetWorldPosition(float x, float y);
	void SetWorldRotation(float degrees);

private:
	Transform* GetParentTransform() const;
};

static constexpr float PI = 3.14159265358979323846f;
static constexpr float DEG_TO_RAD = PI / 180.0f;
static constexpr float RAD_TO_DEG = 180.0f / PI;

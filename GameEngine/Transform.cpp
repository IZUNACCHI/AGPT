// Transform.cpp (Simplified 2D)
#include "Transform.h"
#include "GameObject.h" // Needed for GetComponent, GetParent, etc.

Transform* Transform::GetParentTransform() const {
	if (!owner) return nullptr;

	GameObject* parentObj = owner->GetParent();
	if (!parentObj) return nullptr;

	Transform* parentTransform = parentObj->GetComponent<Transform>();
	if (!parentTransform) return nullptr;

	return parentTransform;
}

// Position Mutators
void Transform::SetLocalPosition(float x, float y) {
	localPosition.x = x;
	localPosition.y = y;
}

void Transform::MoveLocal(float x, float y) {
	// Add the delta directly to the local position
	localPosition.x += x;
	localPosition.y += y;
}

// Rotation Mutators
void Transform::SetLocalRotation(float degrees) {
	// Allows setting the absolute local rotation
	localRotation = degrees;
}

void Transform::RotateLocal(float degrees) {
	// Add the delta to the local rotation
	localRotation += degrees;

	// Optional: Keep rotation within the range [0, 360)
	// localRotation = std::fmod(localRotation, 360.0f);
	// if (localRotation < 0) localRotation += 360.0f;
}

// Scale Mutators
void Transform::SetLocalScale(float x, float y) {
	// Allows setting the absolute local scale
	localScale.x = x;
	localScale.y = y;
}

void Transform::ScaleLocal(float x, float y) {
	// Multiply the current local scale by the factor (x, y)
	localScale.x *= x;
	localScale.y *= y;
}

// --- World Getters ---

Vec2 Transform::GetWorldScale() const {
	Transform* parent = nullptr;
	parent = GetParentTransform();

	// No parent world scale = local scale
	if (!parent)
		return localScale;

	Vec2 parentScale = parent->GetWorldScale();
	return {
		localScale.x * parentScale.x,
		localScale.y * parentScale.y
	};
}

float Transform::GetWorldRotation() const {
	Transform* parent = nullptr;
	parent = GetParentTransform();
	// Recursive rotation application: WorldRotation = ParentWorldRotation + LocalRotation
	return parent ? parent->GetWorldRotation() + localRotation : localRotation;
}

Vec2 Transform::GetWorldPosition() const {
	Transform* parent = nullptr;
	parent = GetParentTransform();
	if (!parent) return localPosition;

	// 1. Apply Parent's World Scale to Local Position
	// This scales the child's position relative to the parent's size.
	Vec2 parentWorldScale = parent->GetWorldScale();
	Vec2 localScaled = {
		localPosition.x * parentWorldScale.x,
		localPosition.y * parentWorldScale.y
	};

	// 2. Apply Parent's World Rotation to the Scaled Local Position
	Vec2 rotated = localScaled.Rotated(parent->GetWorldRotation());

	// 3. Add Parent's World Position
	return parent->GetWorldPosition() + rotated;
}

// --- World Setters ---

void Transform::SetWorldRotation(float degrees) {
	Transform* parent = nullptr;
	parent = GetParentTransform();
	// Local Rotation = World Rotation - Parent World Rotation
	localRotation = parent ? degrees - parent->GetWorldRotation() : degrees;
}

void Transform::SetWorldPosition(const Vec2& position) {
	Transform* parent = nullptr;
	parent = GetParentTransform();
	if (!parent) {
		localPosition = position;
		return;
	}

	// 1. Calculate the offset (dx, dy) in World Space
	Vec2 offset = position - parent->GetWorldPosition();

	// 2. Rotate the offset by the *negative* Parent World Rotation
	// This transforms the vector from World Space back into Parent's Local Space orientation.
	Vec2 rotatedOffset = offset.Rotated(-parent->GetWorldRotation());

	// 3. Divide the rotated offset by the Parent's World Scale
	// This converts the vector from Parent's Local Space (scaled) back to *this* object's Local Position.
	Vec2 parentWorldScale = parent->GetWorldScale();

	// Check for division by zero (safety)
	float scaleX = parentWorldScale.x == 0.0f ? 1.0f : parentWorldScale.x;
	float scaleY = parentWorldScale.y == 0.0f ? 1.0f : parentWorldScale.y;

	localPosition = {
		rotatedOffset.x / scaleX,
		rotatedOffset.y / scaleY
	};
}
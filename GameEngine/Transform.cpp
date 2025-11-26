// Transform.cpp
#include "Transform.h"
#include "GameObject.h"

Transform* Transform::GetParentTransform() const {
	if (!owner || !owner->GetParent()) return nullptr;
	return owner->GetParent()->GetComponent<Transform>();
}

float Transform::GetWorldPositionX() const {
	Transform* parent = GetParentTransform();
	if (!parent) return localPositionX;

	float parentRad = parent->GetWorldRotation() * DEG_TO_RAD;
	float cosR = std::cos(parentRad);
	float sinR = std::sin(parentRad);

	float localScaledX = localPositionX * parent->GetWorldScaleX();
	float localScaledY = localPositionY * parent->GetWorldScaleY();

	float rotatedX = localScaledX * cosR - localScaledY * sinR;
	return parent->GetWorldPositionX() + rotatedX;
}

float Transform::GetWorldPositionY() const {
	Transform* parent = GetParentTransform();
	if (!parent) return localPositionY;

	float parentRad = parent->GetWorldRotation() * DEG_TO_RAD;
	float cosR = std::cos(parentRad);
	float sinR = std::sin(parentRad);

	float localScaledX = localPositionX * parent->GetWorldScaleX();
	float localScaledY = localPositionY * parent->GetWorldScaleY();

	float rotatedY = localScaledX * sinR + localScaledY * cosR;
	return parent->GetWorldPositionY() + rotatedY;
}

float Transform::GetWorldRotation() const {
	Transform* parent = GetParentTransform();
	return parent ? parent->GetWorldRotation() + localRotation : localRotation;
}

float Transform::GetWorldScaleX() const {
	Transform* parent = GetParentTransform();
	return parent ? parent->GetWorldScaleX() * localScaleX : localScaleX;
}

float Transform::GetWorldScaleY() const {
	Transform* parent = GetParentTransform();
	return parent ? parent->GetWorldScaleY() * localScaleY : localScaleY;
}

void Transform::SetWorldPosition(float x, float y) {
	Transform* parent = GetParentTransform();
	if (!parent) {
		localPositionX = x;
		localPositionY = y;
		return;
	}

	float dx = x - parent->GetWorldPositionX();
	float dy = y - parent->GetWorldPositionY();

	float parentRad = -parent->GetWorldRotation() * DEG_TO_RAD;
	float cosR = std::cos(parentRad);
	float sinR = std::sin(parentRad);

	float localX = dx * cosR - dy * sinR;
	float localY = dx * sinR + dy * cosR;

	localPositionX = localX / parent->GetWorldScaleX();
	localPositionY = localY / parent->GetWorldScaleY();
}

void Transform::SetWorldRotation(float degrees) {
	Transform* parent = GetParentTransform();
	localRotation = parent ? degrees - parent->GetWorldRotation() : degrees;
}
#include "Transform.h"

#include <algorithm>
#include <cmath>

#include <box2d/box2d.h>

#include "GameObject.h"
#include "Rigidbody2D.h"
#include "Scene.h"

static void SyncBodyPositionFromTransform(GameObject* go, const Vector2f& worldPos) {
	if (!go) return;

	auto rb = go->GetComponent<Rigidbody2D>();
	if (!rb) return;

	b2BodyId bodyId = rb->GetBodyId();
	if (!b2Body_IsValid(bodyId)) return;

	// Keep current rotation, teleport position.
	const b2Rot rot = b2Body_GetRotation(bodyId);
	b2Body_SetTransform(bodyId, b2Vec2{ worldPos.x, worldPos.y }, rot);
}

static void SyncBodyRotationFromTransform(GameObject* go, float worldRotationDegrees) {
	if (!go) return;

	auto rb = go->GetComponent<Rigidbody2D>();
	if (!rb) return;

	b2BodyId bodyId = rb->GetBodyId();
	if (!b2Body_IsValid(bodyId)) return;

	// Keep current position, teleport rotation.
	const b2Vec2 pos = b2Body_GetPosition(bodyId);
	const float rad = worldRotationDegrees * 3.14159265358979323846f / 180.0f;
	b2Body_SetTransform(bodyId, pos, b2MakeRot(rad));
}

Transform::Transform(GameObject* gameObject)
	: Component("Transform") {
	m_gameObject = gameObject;
}

void Transform::SetPosition(const Vector2f& position) {
	m_localPosition = position;

	// If this object has a Rigidbody2D, keep physics in sync with Transform changes.
	SyncBodyPositionFromTransform(m_gameObject, position);

	SetDirty();
}

void Transform::SetPosition(float x, float y) {
	SetPosition(Vector2f(x, y));
}

Vector2f Transform::GetWorldPosition() const {
	UpdateWorldMatrix();
	return m_worldPosition;
}

void Transform::SetRotation(float rotation) {
	m_localRotation = rotation;

	// If this object has a Rigidbody2D, keep physics in sync with Transform changes.
	SyncBodyRotationFromTransform(m_gameObject, rotation);

	SetDirty();
}

void Transform::SetRotationRadians(float rotation) {
	SetRotation(rotation * 180.0f / 3.14159265358979323846f);
}

float Transform::GetWorldRotation() const {
	UpdateWorldMatrix();
	return m_worldRotation;
}


Vector2f Transform::TransformDirection(const Vector2f& localDir) const {
	// Rotate a direction vector by the transform's world rotation (degrees).
	const float deg = GetWorldRotation();
	const float rad = deg * 3.14159265358979323846f / 180.0f;
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	return Vector2f(localDir.x * c - localDir.y * s, localDir.x * s + localDir.y * c);
}

Vector2f Transform::GetRight() const {
	return TransformDirection(Vector2f(1.0f, 0.0f));
}

Vector2f Transform::GetUp() const {
	return TransformDirection(Vector2f(0.0f, 1.0f));
}

void Transform::SetScale(const Vector2f& scale) {
	m_localScale = scale;
	SetDirty();
}

void Transform::SetScale(float x, float y) {
	m_localScale = Vector2f(x, y);
	SetDirty();
}

void Transform::SetScale(float scale) {
	m_localScale = Vector2f(scale, scale);
	SetDirty();
}

Vector2f Transform::GetWorldScale() const {
	UpdateWorldMatrix();
	return m_worldScale;
}

void Transform::SetParent(Transform* parent) {
	if (m_parent == parent) {
		return;
	}
	if (m_parent) {
		m_parent->RemoveChild(this);
	}
	m_parent = parent;
	if (m_parent) {
		m_parent->AddChild(this);
	}
	if (m_gameObject) {
		if (auto* scene = m_gameObject->GetScene()) {
			scene->UpdateRootGameObject(m_gameObject);
		}
		m_gameObject->UpdateActiveInHierarchy();
	}
	SetDirty();
}

void Transform::AddChild(Transform* child) {
	if (!child) {
		return;
	}
	if (std::find(m_children.begin(), m_children.end(), child) == m_children.end()) {
		m_children.push_back(child);
	}
}

void Transform::RemoveChild(Transform* child) {
	auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end()) {
		m_children.erase(it);
	}
}

Transform* Transform::GetChild(size_t index) const {
	if (index >= m_children.size()) {
		return nullptr;
	}
	return m_children[index];
}

void Transform::Translate(const Vector2f& translation) {
	SetPosition(m_localPosition + translation);
}

void Transform::Translate(float x, float y) {
	Translate(Vector2f(x, y));
}

void Transform::Rotate(float angle) {
	SetRotation(m_localRotation + angle);
}

Matrix3x3f Transform::GetLocalMatrix() const {
	Matrix3x3f translation = Matrix3x3f::Translation(m_localPosition);
	// Matrix3x3f::Rotation expects degrees and handles Deg->Rad internally.
	Matrix3x3f rotation = Matrix3x3f::Rotation(m_localRotation);
	Matrix3x3f scale = Matrix3x3f::Scale(m_localScale);
	return translation * rotation * scale;
}

Matrix3x3f Transform::GetWorldMatrix() const {
	UpdateWorldMatrix();
	return m_worldMatrix;
}

void Transform::SetDirty() {
	m_hasChanged = true;
	m_worldMatrixDirty = true;

	for (auto* child : m_children) {
		if (child) {
			child->SetDirty();
		}
	}
}

void Transform::UpdateWorldMatrix() const {
	if (!m_worldMatrixDirty) {
		return;
	}

	if (m_parent) {
		m_worldMatrix = m_parent->GetWorldMatrix() * GetLocalMatrix();
	}
	else {
		m_worldMatrix = GetLocalMatrix();
	}

	m_worldPosition = m_worldMatrix * Vector2f::Zero();
	m_worldRotation = m_localRotation + (m_parent ? m_parent->GetWorldRotation() : 0.0f);

	if (m_parent) {
		auto parentScale = m_parent->GetWorldScale();
		m_worldScale = Vector2f(m_localScale.x * parentScale.x, m_localScale.y * parentScale.y);
	}
	else {
		m_worldScale = m_localScale;
	}

	m_worldMatrixDirty = false;
	m_hasChanged = false;
}

std::shared_ptr<Component> Transform::Clone() const {
	auto clone = std::make_shared<Transform>(nullptr);
	clone->SetPosition(m_localPosition);
	clone->SetRotation(m_localRotation);
	clone->SetScale(m_localScale);
	return clone;
}

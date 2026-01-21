#include "Transform.h"
#include "GameObject.h"
#include "Scene.h"
#include <algorithm>
#include <cmath>

Transform::Transform(GameObject* gameObject): Component("Transform") {
	m_gameObject = gameObject;
}

void Transform::SetPosition(const Vector2f& position) {
	m_localPosition = position;
	SetDirty();
}

void Transform::SetPosition(float x, float y) {
	m_localPosition = Vector2f(x, y);
	SetDirty();
}

Vector2f Transform::GetWorldPosition() const {
	UpdateWorldMatrix();
	return m_worldPosition;
}

void Transform::SetRotation(float rotation) {
	m_localRotation = rotation;
	SetDirty();
}

void Transform::SetRotationRadians(float rotation) {
	m_localRotation = rotation * 180.0f / 3.14159265358979323846f;
	SetDirty();
}

float Transform::GetWorldRotation() const {
	UpdateWorldMatrix();
	return m_worldRotation;
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
	m_localPosition += translation;
	SetDirty();
}

void Transform::Translate(float x, float y) {
	m_localPosition += Vector2f(x, y);
	SetDirty();
}

void Transform::Rotate(float angle) {
	m_localRotation += angle;
	SetDirty();
}

Matrix3x3f Transform::GetLocalMatrix() const {
	Matrix3x3f translation = Matrix3x3f::Translation(m_localPosition);
	Matrix3x3f rotation = Matrix3x3f::Rotation(m_localRotation * 3.14159265358979323846f / 180.0f);
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

	m_worldPosition = m_worldMatrix.TransformPoint(Vector2f::Zero());
	m_worldRotation = m_localRotation + (m_parent ? m_parent->GetWorldRotation() : 0.0f);
	m_worldScale = m_localScale * (m_parent ? m_parent->GetWorldScale() : Vector2f::One());

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
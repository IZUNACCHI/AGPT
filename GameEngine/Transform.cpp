#include "Transform.h"
#include "GameObject.h"
#include <cmath>

	Transform::Transform(GameObject* gameObject)
		: m_gameObject(gameObject) {
		m_worldMatrix = Matrix3x3f::Identity();
	}

	Transform::~Transform() {
		// Remove from parent
		if (m_parent) {
			m_parent->RemoveChild(this);
		}

		// Orphan children
		for (auto child : m_children) {
			child->m_parent = nullptr;
		}
	}

	void Transform::SetPosition(const Vector2f& position) {
		if (m_localPosition == position) return;

		m_localPosition = position;
		SetDirty();
	}

	void Transform::SetPosition(float x, float y) {
		SetPosition(Vector2f(x, y));
	}

	Vector2f Transform::GetWorldPosition() const {
		if (m_worldMatrixDirty) {
			UpdateWorldMatrix();
		}
		return m_worldPosition;
	}

	void Transform::SetRotation(float rotation) {
		m_localRotation = rotation;
		SetDirty();
	}

	void Transform::SetRotationRadians(float rotation) {
		SetRotation(rotation * Math::Constants<float>::Rad2Deg);
	}

	float Transform::GetWorldRotation() const {
		if (m_worldMatrixDirty) {
			UpdateWorldMatrix();
		}
		return m_worldRotation;
	}

	void Transform::SetScale(const Vector2f& scale) {
		m_localScale = scale;
		SetDirty();
	}

	void Transform::SetScale(float x, float y) {
		SetScale(Vector2f(x, y));
	}

	void Transform::SetScale(float scale) {
		SetScale(Vector2f(scale, scale));
	}

	Vector2f Transform::GetWorldScale() const {
		if (m_worldMatrixDirty) {
			UpdateWorldMatrix();
		}
		return m_worldScale;
	}

	void Transform::SetParent(Transform* parent) {
		if (m_parent == parent) return;

		// Remove from current parent
		if (m_parent) {
			m_parent->RemoveChild(this);
		}

		m_parent = parent;

		// Add to new parent
		if (m_parent) {
			m_parent->AddChild(this);
		}

		SetDirty();
	}

	void Transform::AddChild(Transform* child) {
		if (child == nullptr || child == this) return;

		// Remove child from its current parent
		if (child->m_parent && child->m_parent != this) {
			child->m_parent->RemoveChild(child);
		}

		m_children.push_back(child);
		child->m_parent = this;
		child->SetDirty();
	}

	void Transform::RemoveChild(Transform* child) {
		auto it = std::find(m_children.begin(), m_children.end(), child);
		if (it != m_children.end()) {
			m_children.erase(it);
			child->m_parent = nullptr;
			child->SetDirty();
		}
	}

	Transform* Transform::GetChild(size_t index) const {
		if (index < m_children.size()) {
			return m_children[index];
		}
		return nullptr;
	}

	void Transform::Translate(const Vector2f& translation) {
		m_localPosition += translation;
		SetDirty();
	}

	void Transform::Translate(float x, float y) {
		Translate(Vector2f(x, y));
	}

	void Transform::Rotate(float angle) {
		m_localRotation += angle;
		// Normalize rotation
		m_localRotation = fmodf(m_localRotation, 360.0f);
		if (m_localRotation < 0) m_localRotation += 360.0f;
		SetDirty();
	}

	Matrix3x3f Transform::GetLocalMatrix() const {
		return Matrix3x3f::Translation(m_localPosition) *
			Matrix3x3f::Rotation(m_localRotation) *
			Matrix3x3f::Scale(m_localScale);
	}

	Matrix3x3f Transform::GetWorldMatrix() const {
		if (m_worldMatrixDirty) {
			UpdateWorldMatrix();
		}
		return m_worldMatrix;
	}

	void Transform::SetDirty() {
		m_hasChanged = true;
		m_worldMatrixDirty = true;

		// Mark children as dirty
		for (auto child : m_children) {
			child->SetDirty();
		}
	}

	void Transform::UpdateWorldMatrix() const {
		if (!m_worldMatrixDirty) return;

		if (m_parent) {
			Matrix3x3f parentMatrix = m_parent->GetWorldMatrix();
			Matrix3x3f localMatrix = GetLocalMatrix();
			m_worldMatrix = parentMatrix * localMatrix;

			// Extract world position from matrix
			m_worldPosition = Vector2f(m_worldMatrix.m[6], m_worldMatrix.m[7]);

			// For 2D: world rotation is parent rotation + local rotation
			m_worldRotation = m_parent->GetWorldRotation() + m_localRotation;

			// World scale is parent scale * local scale
			Vector2f parentScale = m_parent->GetWorldScale();
			m_worldScale = Vector2f(parentScale.x * m_localScale.x,
				parentScale.y * m_localScale.y);
		}
		else {
			m_worldMatrix = GetLocalMatrix();
			m_worldPosition = m_localPosition;
			m_worldRotation = m_localRotation;
			m_worldScale = m_localScale;
		}

		m_worldMatrixDirty = false;
	}

#pragma once

#include <vector>
#include "Component.h"
#include "Types.hpp"

class GameObject;

/// Component that stores position, rotation, scale, and hierarchy.
class Transform : public Component {
public:
	/// Creates a Transform for the given GameObject.
	explicit Transform(GameObject* gameObject);
	/// Destroys the Transform.
	~Transform() override = default;

	/// Sets the local position.
	void SetPosition(const Vector2f& position);
	/// Sets the local position.
	void SetPosition(float x, float y);
	/// Returns the local position.
	Vector2f GetPosition() const { return m_localPosition; }
	/// Returns the world position.
	Vector2f GetWorldPosition() const;

	/// Sets the local rotation in degrees.
	void SetRotation(float rotation);
	/// Sets the local rotation in radians.
	void SetRotationRadians(float rotation);
	/// Returns the local rotation in degrees.
	float GetRotation() const { return m_localRotation; }
	/// Returns the world rotation in degrees.
	float GetWorldRotation() const;

	/// Sets the local scale.
	void SetScale(const Vector2f& scale);
	/// Sets the local scale.
	void SetScale(float x, float y);
	/// Sets the local uniform scale.
	void SetScale(float scale);
	/// Returns the local scale.
	Vector2f GetScale() const { return m_localScale; }
	/// Returns the world scale.
	Vector2f GetWorldScale() const;

	/// Sets the parent Transform.
	void SetParent(Transform* parent);
	/// Returns the parent Transform.
	Transform* GetParent() const { return m_parent; }

	/// Adds a child Transform.
	void AddChild(Transform* child);
	/// Removes a child Transform.
	void RemoveChild(Transform* child);
	/// Returns the child list.
	const std::vector<Transform*>& GetChildren() const { return m_children; }
	/// Returns the number of children.
	size_t GetChildCount() const { return m_children.size(); }
	/// Returns the child at the provided index.
	Transform* GetChild(size_t index) const;

	/// Translates by a local vector.
	void Translate(const Vector2f& translation);
	/// Translates by a local vector.
	void Translate(float x, float y);
	/// Rotates by the provided angle in degrees.
	void Rotate(float angle);

	/// Returns the local transform matrix.
	Matrix3x3f GetLocalMatrix() const;
	/// Returns the world transform matrix.
	Matrix3x3f GetWorldMatrix() const;

	/// Returns true if the transform changed.
	bool HasChanged() const { return m_hasChanged; }
	/// Marks the transform as dirty.
	void SetDirty();

	/// Clones the transform values.
	std::shared_ptr<Component> Clone() const override;

private:
	/// Updates the cached world matrix.
	void UpdateWorldMatrix() const;

	/// Local position.
	Vector2f m_localPosition = Vector2f::Zero();
	/// Local rotation in degrees.
	float m_localRotation = 0.0f;
	/// Local scale.
	Vector2f m_localScale = Vector2f::One();

	/// Cached world position.
	mutable Vector2f m_worldPosition = Vector2f::Zero();
	/// Cached world rotation in degrees.
	mutable float m_worldRotation = 0.0f;
	/// Cached world scale.
	mutable Vector2f m_worldScale = Vector2f::One();
	/// Cached world matrix.
	mutable Matrix3x3f m_worldMatrix;

	/// Parent transform.
	Transform* m_parent = nullptr;
	/// Child transforms.
	std::vector<Transform*> m_children;

	/// Dirty flag for local changes.
	mutable bool m_hasChanged = true;
	/// Dirty flag for world matrix cache.
	mutable bool m_worldMatrixDirty = true;
};
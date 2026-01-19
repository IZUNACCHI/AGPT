#pragma once

#include <memory>
#include <vector>
#include "Types.hpp"

class GameObject;

class Transform {
public:
	Transform(GameObject* gameObject);
	~Transform();

	// Position
	void SetPosition(const Vector2f& position);
	void SetPosition(float x, float y);
	Vector2f GetPosition() const { return m_localPosition; }
	Vector2f GetWorldPosition() const;

	// Rotation (in degrees for 2D)
	void SetRotation(float rotation);
	void SetRotationRadians(float rotation);
	float GetRotation() const { return m_localRotation; }
	float GetWorldRotation() const;

	// Scale
	void SetScale(const Vector2f& scale);
	void SetScale(float x, float y);
	void SetScale(float scale);
	Vector2f GetScale() const { return m_localScale; }
	Vector2f GetWorldScale() const;

	// Parenting
	void SetParent(Transform* parent);
	Transform* GetParent() const { return m_parent; }

	// Children management
	void AddChild(Transform* child);
	void RemoveChild(Transform* child);
	const std::vector<Transform*>& GetChildren() const { return m_children; }
	size_t GetChildCount() const { return m_children.size(); }
	Transform* GetChild(size_t index) const;

	// Transform operations
	void Translate(const Vector2f& translation);
	void Translate(float x, float y);
	void Rotate(float angle);

	// Transform matrices
	Matrix3x3f GetLocalMatrix() const;
	Matrix3x3f GetWorldMatrix() const;

	// Getters
	GameObject* GetGameObject() const { return m_gameObject; }
	bool HasChanged() const { return m_hasChanged; }

	// Marks transform as dirty (needs matrix recomputation)
	void SetDirty();

private:
	void UpdateWorldMatrix() const;

	GameObject* m_gameObject;

	// Local transform
	Vector2f m_localPosition = Vector2f::Zero();
	float m_localRotation = 0.0f;  // In degrees
	Vector2f m_localScale = Vector2f::One();

	// World transform (cached)
	mutable Vector2f m_worldPosition = Vector2f::Zero();
	mutable float m_worldRotation = 0.0f;
	mutable Vector2f m_worldScale = Vector2f::One();
	mutable Matrix3x3f m_worldMatrix;

	// Hierarchy
	Transform* m_parent = nullptr;
	std::vector<Transform*> m_children;

	// Dirty flags
	mutable bool m_hasChanged = true;
	mutable bool m_worldMatrixDirty = true;
};
#pragma once

class GameObject;

class Component {
public:
	virtual ~Component() = default;

	virtual void Start() {}
	virtual void Update() {}

	GameObject* GetOwner() const { return owner; }

protected:
	friend class GameObject;
	GameObject* owner = nullptr;
};

#pragma once

class GameObject;

class Component {
public:
	virtual ~Component() = default;

	virtual void Start() {}
	virtual void Update(float deltaTime) {}

	GameObject* GetOwner() const { return owner; }
	bool IsActive() const { return active; }

protected:
	friend class GameObject;
	GameObject* owner = nullptr;
private:
	bool active;
};

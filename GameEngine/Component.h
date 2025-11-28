#pragma once
#include "UniqueID.h"
class GameObject;

class Component {
public:
	Component(bool startActive = true) : active(startActive), id(Engine::GenerateUniqueId()) {
		name = "Component" + GetId();
	}
	virtual ~Component() = default;

	virtual void Start() {}
	virtual void Update(float deltaTime) {}

	GameObject* GetOwner() const { return owner; }
	bool IsActive() const { return active; }
	void SetActive(bool isActive) { active = isActive; }

	std::string name;
	uint64_t GetId() const { return id; }

protected:
	friend class GameObject;
	GameObject* owner = nullptr;
private:
	bool active;
	uint64_t id;
};

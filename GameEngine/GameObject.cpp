#include "GameObject.h"

void GameObject::SetParent(GameObject* newParent) {
	if (parent == newParent) return;

	RemoveFromParent();

	parent = newParent;
	if (parent) {
		parent->children.push_back(std::unique_ptr<GameObject>(this));
	}
}

GameObject* GameObject::AddChild(const std::string& childName) {
	auto child = std::make_unique<GameObject>(childName);
	GameObject* ptr = child.get();
	child->parent = this;
	children.push_back(std::move(child));
	return ptr;
}

void GameObject::RemoveFromParent() {
	if (!parent) return;

	auto& parentChildren = parent->children;
	auto it = std::find_if(parentChildren.begin(), parentChildren.end(),
		[this](const auto& c) { return c.get() == this; });
	if (it != parentChildren.end()) {
		parentChildren.erase(it);
	}
	parent = nullptr;
}

void GameObject::Update() {
	for (auto& comp : components) {
		comp->Update();
	}

	for (auto& child : children) {
		child->Update();
	}
}


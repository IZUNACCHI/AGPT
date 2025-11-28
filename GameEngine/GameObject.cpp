#include "GameObject.h"
#include <iostream>

void GameObject::SetParent(GameObject* newParent = nullptr)
{
	//self check
	if (parent == newParent) return;

	// remove from old parent's children list 
	if (parent)
	{
		auto& siblings = parent->children;
		siblings.erase(
			std::remove(siblings.begin(), siblings.end(), this),
			siblings.end()
		);
	}

	// set new parent
	parent = newParent;

	// add to new parent's children list
	if (parent)
		parent->children.push_back(this);
}

void GameObject::AddChild(GameObject* child)
{
	// null or self check
	if (!child || child == this) return;

	// check for cycles
	GameObject* p = this;
	while (p)
	{
		if (p == child) return; // child is already an ancestor
		p = p->parent;
	}

	// scene check
	if (child->owningScene != owningScene) return;

	// set child's parent to this
	child->SetParent(this);
}

void GameObject::RemoveChild(GameObject* child)
{
	//null check
	if (!child) return;
	if (child->parent != this) return; // not a child of this

	child->SetParent(nullptr);// detach from parent
}

void GameObject::RemoveFromParent()
{
	// null check
	if (!parent) return;
	// detach from parent
	SetParent(nullptr);
}

void GameObject::OnInit()
{

}

void GameObject::OnUpdate(float deltaTime)
{

}

void GameObject::OnDestroy()
{

}


void GameObject::Update(float deltaTime)
{
	OnUpdate(deltaTime);
	// Update active components
	for (auto& comp : components)
		if (comp->IsActive()) {
			comp->Update(deltaTime);
		}

	// Update active children
	for (auto& child : children)
	{
		if (child->active)          // skip inactive children
			child->Update(deltaTime);
	}
}
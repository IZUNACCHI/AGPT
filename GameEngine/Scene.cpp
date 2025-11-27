#include "Scene.h"
#include "FRect.hpp"       

Scene::Scene() : instanceId(Engine::GenerateUniqueId()){}


Scene::~Scene()
{
	objects.clear();           // destroys everything 
}

GameObject* Scene::CreateGameObject(const std::string& name /*= "GameObject"*/)
{
	// Create GameObject 
	auto obj = std::make_unique<GameObject>(name);
	// Set owning scene
	obj->owningScene = this;
	// Every GameObject must have a Transform component for now
	obj->AddComponent<Transform>();
	// Return raw pointer but keep ownership in the scene
	GameObject* ptr = obj.get();
	objects.emplace_back(std::move(obj)); 
	return ptr;
}

//
void Scene::Destroy(GameObject* go, bool recursive = false)
{
	// null check
	if (!go) return;

	// Detach from parent
	go->RemoveFromParent();

	// If recursive, destroy children
	if (recursive) {
		for (GameObject* child : go->GetChildren())
			Destroy(child);
	}
	else {
		// If not recursive, detach children
		for (GameObject* child : go->GetChildren())
					go->RemoveChild(child);
	}

	// Find and remove from objects list
	auto it = std::find_if(objects.begin(), objects.end(),
		[go](const std::unique_ptr<GameObject>& ptr) { return ptr.get() == go; });

	// If found, erase it (this will delete the GameObject)
	if (it != objects.end())
		objects.erase(it);
}

void Scene::CollectRenderItems(GameObject* go)
{
	// Skip inactive or invisible objects
	if (!go->IsActive() || !go->visible) return;

	// Check for SpriteRenderer component
	if (auto sr = go->GetComponent<SpriteRenderer>())
	{
		if (sr->texture)
		{
			if (auto tr = go->GetComponent<Transform>())
			{
				renderQueue.push_back({ sr, tr, sr->sortingLayer, sr->sortingOrder });
			}
		}
	}

	for (auto& child : go->GetChildren())
		CollectRenderItems(child);
}

void Scene::Update(float deltaTime)
{
	for (const auto& obj : objects)
	{
		if (!obj->GetParent() && obj->active)   // start only from roots
			obj->Update(deltaTime);
	}
}

void Scene::Render(Renderer& renderer)
{
	renderQueue.clear();

	for (const auto& obj : objects)
	{
		if (!obj->GetParent())
			CollectRenderItems(obj.get());
	}

	std::sort(renderQueue.begin(), renderQueue.end());

	for (const RenderItem& item : renderQueue)
	{
		Transform* tr = item.transform;
		SpriteRenderer* sr = item.sr;

		float wx = tr->GetWorldPositionX();
		float wy = tr->GetWorldPositionY();
		float rot = tr->GetWorldRotation();           // degrees
		float sx = tr->GetWorldScaleX();
		float sy = tr->GetWorldScaleY();

		Texture* tex = sr->texture;
		float texW = static_cast<float>(tex->Width());
		float texH = static_cast<float>(tex->Height());

		FRect clip = sr->clipRect;

		// if clip is zero treat as full texture
		if (clip.w <= 0.0f || clip.h <= 0.0f)
			clip = { 0.0f, 0.0f, texW, texH };

		float drawW = clip.w * sx;
		float drawH = clip.h * sy;

		// Pivot = center of the sprite (very common for 2D games)
		FRect dst{
			wx - drawW * 0.5f,
			wy - drawH * 0.5f,
			drawW,
			drawH
		};

		const FRect* srcPtr = (sr->clipRect.w <= 0.0f || sr->clipRect.h <= 0.0f) ? nullptr : &clip;

		renderer.DrawTexture(*tex, srcPtr, dst, rot, sr->flip);
	}
}
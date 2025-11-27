#pragma once

#include "GameObject.h"
#include "Renderer.h"
#include "Transform.h"
#include "SpriteRenderer.h"

#include <vector>
#include <algorithm>

class Scene
{
public:
	Scene();
	~Scene();

	// Create a new GameObject (automatically adds a Transform component - every object without Transform cannot be rendered)
	GameObject* CreateGameObject(const std::string& name = "GameObject");

	void Update(float deltaTime);
	void Render(Renderer& renderer);
	void Destroy(GameObject* go, bool recursive);

private:
	std::vector<std::unique_ptr<GameObject>> objects;

	struct RenderItem
	{
		SpriteRenderer* sr;
		Transform* transform;

		int sortingLayer = 0;     // smaller = drawn earlier (background = low values, foreground = high values)
		int sortingOrder = 0;     // higher value = drawn on top within the same layer

		bool operator<(const RenderItem& other) const
		{
			if (sortingLayer != other.sortingLayer) return sortingLayer < other.sortingLayer;
			return sortingOrder < other.sortingOrder;
		}
	};

	std::vector<RenderItem> renderQueue;

	void CollectRenderItems(GameObject* go);
};

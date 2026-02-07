#include "RenderQueue.h"

#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "GameObject.h"
#include "Transform.h"

#include <algorithm>
#include <unordered_map>

namespace {
	RenderQueue::SortOptions g_spriteSortOptions{};
}

void RenderQueue::Clear() {
	m_items.clear();
}

void RenderQueue::SetSpriteSortOptions(const SortOptions& options) {
	g_spriteSortOptions = options;
}

RenderQueue::SortOptions RenderQueue::GetSpriteSortOptions() {
	return g_spriteSortOptions;
}

float RenderQueue::ResolveAxisValue(const SpriteRenderer* sprite, SortAxis axis) {
	if (!sprite) {
		return 0.0f;
	}
	if (axis == SortAxis::None) {
		return 0.0f;
	}
	auto* tr = sprite->GetTransform();
	if (!tr) {
		return 0.0f;
	}
	const Vector2f pos = tr->GetWorldPosition();
	return (axis == SortAxis::X) ? pos.x : pos.y;
}

void RenderQueue::Add(const SpriteRenderer* sprite) {
	if (!sprite) {
		return;
	}
	auto* go = sprite->GetGameObject();
	if (!go) {
		return;
	}

	Item it;
	it.type = ItemType::Sprite;
	it.ptr = sprite;
	it.layer = go->GetLayer();
	it.layerOrder = sprite->GetLayerOrder();
	it.primaryAxis = ResolveAxisValue(sprite, g_spriteSortOptions.primaryAxis);
	it.secondaryAxis = ResolveAxisValue(sprite, g_spriteSortOptions.secondaryAxis);
	it.componentIndex = sprite->GetComponentIndex();
	it.instanceId = sprite->GetInstanceID();
	m_items.push_back(it);
}

void RenderQueue::Add(const TextRenderer* text) {
	if (!text) {
		return;
	}
	auto* go = text->GetGameObject();
	if (!go) {
		return;
	}

	Item it;
	it.type = ItemType::Text;
	it.ptr = text;
	it.layer = go->GetLayer();
	it.layerOrder = text->GetLayerOrder();
	it.primaryAxis = 0.0f;
	it.secondaryAxis = 0.0f;
	it.componentIndex = text->GetComponentIndex();
	it.instanceId = text->GetInstanceID();
	m_items.push_back(it);
}

void RenderQueue::Execute(Renderer& renderer) const {

	//bucket by (layer, layerOrder) then only sort inside buckets.

	// Define a bucket structure.
	struct Bucket {
		int layer = 0;
		int layerOrder = 0;
		std::vector<const Item*> items;
	};

	// Function to create a unique key for each (layer, layerOrder) pair.
	auto makeKey = [](int layer, int layerOrder) -> long long {
		// Pack two 32-bit ints into a single 64-bit key.
		const unsigned long long a = (unsigned long long)(uint32_t)layer;
		const unsigned long long b = (unsigned long long)(uint32_t)layerOrder;
		return (long long)((a << 32) | b);
	};

	std::vector<Bucket> buckets;
	buckets.reserve(m_items.size());

	std::unordered_map<long long, size_t> bucketIndex;
	bucketIndex.reserve(m_items.size());

	// Distribute items into buckets.
	for (const auto& it : m_items) {
		const long long key = makeKey(it.layer, it.layerOrder);
		auto found = bucketIndex.find(key);
		// If not found, create a new bucket.
		if (found == bucketIndex.end()) {
			Bucket b;
			b.layer = it.layer;
			b.layerOrder = it.layerOrder;
			b.items.push_back(&it);
			buckets.push_back(std::move(b));
			bucketIndex.emplace(key, buckets.size() - 1);
		}
		else {
			buckets[found->second].items.push_back(&it);
		}
	}

	// Sort buckets by (layer, layerOrder).
	std::sort(buckets.begin(), buckets.end(), [](const Bucket& a, const Bucket& b) {
		if (a.layer != b.layer) {
			return a.layer < b.layer;
		}
		return a.layerOrder < b.layerOrder;
	});

	const bool axisEnabled = (g_spriteSortOptions.primaryAxis != SortAxis::None) || (g_spriteSortOptions.secondaryAxis != SortAxis::None);

	// Render each bucket.
	for (auto& bucket : buckets) {
		if (bucket.items.size() > 1) {
			std::sort(bucket.items.begin(), bucket.items.end(), [&](const Item* a, const Item* b) {
				// Optional sprite-only axis sort as a tie-break (only when both are sprites).
				if (axisEnabled && a->type == ItemType::Sprite && b->type == ItemType::Sprite) {
					if (a->primaryAxis != b->primaryAxis) {
						return g_spriteSortOptions.primaryAscending ? (a->primaryAxis < b->primaryAxis)
							: (a->primaryAxis > b->primaryAxis);
					}
					if (a->secondaryAxis != b->secondaryAxis) {
						return g_spriteSortOptions.secondaryAscending ? (a->secondaryAxis < b->secondaryAxis)
							: (a->secondaryAxis > b->secondaryAxis);
					}
				}

				// Deterministic fallbacks (cross-type safe).
				if (a->componentIndex != b->componentIndex) {
					return a->componentIndex < b->componentIndex;
				}
				return a->instanceId < b->instanceId;
			});
		}

		for (const Item* it : bucket.items) {
			if (!it) continue;
			switch (it->type) {
			case ItemType::Sprite: {
				auto* sprite = static_cast<const SpriteRenderer*>(it->ptr);
				if (sprite) {
					sprite->Render(renderer);
				}
				break;
			}
			case ItemType::Text: {
				auto* text = static_cast<const TextRenderer*>(it->ptr);
				if (text) {
					text->Render(renderer);
				}
				break;
			}
			default:
				break;
			}
		}
	}
}

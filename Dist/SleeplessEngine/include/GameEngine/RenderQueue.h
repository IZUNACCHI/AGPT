#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

class Renderer;
class SpriteRenderer;
class TextRenderer;

// Manages a render queue for SpriteRenderer and TextRenderer components
class RenderQueue {
public:
	// Sort axis used only by SpriteRenderer tie-breaking
	enum class SortAxis {
		None,
		X,
		Y
	};

	struct SortOptions {
		SortAxis primaryAxis = SortAxis::None;
		SortAxis secondaryAxis = SortAxis::None;
		bool primaryAscending = true;
		bool secondaryAscending = true;
	};

	RenderQueue() = default;

	void Clear();

	void Add(const SpriteRenderer* sprite);
	void Add(const TextRenderer* text);

	// Sort then render everything in order.
	void Execute(Renderer& renderer) const;

	// Sprite axis sort configuration (used only as a final tie-breaker
	// after layer + layerOrder).
	static void SetSpriteSortOptions(const SortOptions& options);
	static SortOptions GetSpriteSortOptions();

private:
	enum class ItemType : uint8_t {
		Sprite,
		Text
	};

	struct Item {
		ItemType type{};
		const void* ptr = nullptr; // SpriteRenderer* or TextRenderer*

		int layer = 0;
		int layerOrder = 0;
		float primaryAxis = 0.0f;
		float secondaryAxis = 0.0f;
		size_t componentIndex = 0;
		uint32_t instanceId = 0;
	};

	static float ResolveAxisValue(const SpriteRenderer* sprite, SortAxis axis);

	std::vector<Item> m_items;
};

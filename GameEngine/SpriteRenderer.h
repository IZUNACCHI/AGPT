#pragma once

#include "Component.h"
#include "Texture.h"
#include "Types.hpp"

class Renderer;

/// Component that renders a sprite frame from a BMP-backed texture.
class SpriteRenderer : public Component {
public:
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

	SpriteRenderer();
	explicit SpriteRenderer(Texture* texture);
	~SpriteRenderer() override = default;

	void SetTexture(Texture* texture);
	Texture* GetTexture() const { return m_texture; }

	void SetFrameSize(const Vector2i& size);
	Vector2i GetFrameSize() const { return m_frameSize; }

	void SetFrameIndex(int index);
	int GetFrameIndex() const { return m_frameIndex; }

	void SetLayerOrder(int order) { m_layerOrder = order; }
	int GetLayerOrder() const { return m_layerOrder; }

	static void SetSortOptions(const SortOptions& options);
	static SortOptions GetSortOptions();

	static void RenderAll(Renderer& renderer);

	std::shared_ptr<Component> Clone() const override;

private:
	void Render(Renderer& renderer) const;
	Vector2i GetResolvedFrameSize() const;
	int GetMaxFrames(const Vector2i& frameSize) const;

	Texture* m_texture = nullptr;
	Vector2i m_frameSize = Vector2i::Zero();
	int m_frameIndex = 0;
	int m_layerOrder = 0;
};

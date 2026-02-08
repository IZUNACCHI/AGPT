#pragma once

#include <GameEngine/GameEngine.h>
#include "XenonAssetKeys.h"

// Moves the owning GameObject at a constant speed in a WORLD direction.
// Rotation does not affect the movement direction.
class ParallaxMover2D : public MonoBehaviour {
public:
	ParallaxMover2D() : MonoBehaviour() {
		SetComponentName("ParallaxMover2D");
	}

	void Awake() override {
		t = GetTransform();
	}
	void SetDirection(const Vector2f& dir) {
		m_direction = dir;
		if (m_direction.LengthSquared() > 0.0f) m_direction = m_direction.Normalized();
	}
	void SetSpeed(float pixelsPerSecond) { m_speed = pixelsPerSecond; }

	void Update() override {
		
		if (!t) return;
		if (m_speed == 0.0f) return;
		if (m_direction.LengthSquared() <= 0.0f) return;

		t->SetPosition(t->GetWorldPosition() + m_direction * (m_speed * Time::DeltaTime()));
	}

private:
	Vector2f m_direction = Vector2f(-1.0f, 0.0f);
	float m_speed = 20.0f;
	Transform* t;
};

// Builds a rectangle out of tiles from Blocks.bmp (16 columns).
// Parameters are specified in FRAME coordinates (1-based) to avoid pixel math.
// Because SpriteRenderer renders a single frame, we create a mosaic of many tiles.
inline GameObject* CreateBlocksRect(Scene* scene,
	const std::string& name,
	int row1Based,
	int colStart1Based,
	int colEnd1Based,
	int rowsTall,
	const Vector2f& worldPosition,
	int layerOrder = -9)
{
	if (!scene) return nullptr;

	const int tilesPerRow = 16;
	const int tileW = 512 / tilesPerRow; // 32
	const Vector2i tileSize(tileW, tileW);

	Texture* tex = LoadTexture(XenonAssetKeys::Files::BlocksBmp, Vector3i(255, 0, 255));
	if (!tex) return nullptr;

	auto parent = scene->CreateGameObject<GameObject>(name).get();
	parent->GetTransform()->SetPosition(worldPosition);

	const int row0 = std::max(0, row1Based - 1);
	const int c0 = std::max(0, colStart1Based - 1);
	const int c1 = std::max(c0, colEnd1Based - 1);

	int tileIndex = 0;
	for (int ry = 0; ry < rowsTall; ++ry) {
		for (int cx = c0; cx <= c1; ++cx) {
			const int frameIndex = (row0 + ry) * tilesPerRow + cx;

			auto tile = scene->CreateGameObject<GameObject>(name + "_t" + std::to_string(tileIndex++));
			tile->GetTransform()->SetParent(parent->GetTransform());
			// Layout: build rightwards and downward in world space (negative Y for "down")
			tile->GetTransform()->SetPosition(Vector2f(
				(float)((cx - c0) * tileSize.x),
				(float)(-(ry) * tileSize.y)
			));

			auto sr = tile->AddComponent<SpriteRenderer>();
			sr->SetTexture(tex);
			sr->SetFrameSize(tileSize);
			sr->SetFrameIndex(frameIndex);
			sr->SetLayerOrder(layerOrder);
		}
	}

	return parent;
}

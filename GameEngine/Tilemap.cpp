#include "Tilemap.h"

#include <algorithm>

#include "GameObject.h"
#include "Scene.h"
#include "SpriteRenderer.h"
#include "Texture.h"
#include "Transform.h"
#include "Object.h"

Tilemap::Tilemap() : MonoBehaviour() {
	SetComponentName("Tilemap");
}

void Tilemap::SetTileset(Texture* texture, const Vector2i& tileSize, int tilesPerRow) {
	m_texture = texture;
	m_tileSize = tileSize;
	m_tilesPerRow = std::max(1, tilesPerRow);
}

void Tilemap::SetMapSize(int width, int height) {
	m_width = std::max(0, width);
	m_height = std::max(0, height);
}

void Tilemap::SetTiles(const std::vector<int>& tiles) {
	m_tiles = tiles;
}

void Tilemap::Start() {
	if (m_buildOnStart) {
		Rebuild();
	}
}

void Tilemap::Rebuild() {
	DestroySpawned();
	BuildInternal();
}

void Tilemap::Clear() {
	DestroySpawned();
}

void Tilemap::DestroySpawned() {
	for (auto& w : m_spawned) {
		if (auto s = w.lock()) {
			Object::Destroy(s);
		}
	}
	m_spawned.clear();
}

Vector2f Tilemap::TileLocalCenter(int x, int y) const {
	// Map centered around owner; (0,0) is top-left.
	const float mapW = (float)m_width * (float)m_tileSize.x;
	const float mapH = (float)m_height * (float)m_tileSize.y;

	const float left = -mapW * 0.5f;
	const float top = mapH * 0.5f;

	const float cx = left + ((float)x + 0.5f) * (float)m_tileSize.x;
	const float cy = top - ((float)y + 0.5f) * (float)m_tileSize.y;
	return Vector2f(cx, cy);
}

void Tilemap::BuildInternal() {
	GameObject* owner = GetGameObject();
	if (!owner) return;
	Scene* scene = owner->GetScene();
	if (!scene) return;
	Transform* parent = owner->GetTransform();
	if (!parent) return;

	if (!m_texture || !m_texture->IsValid()) return;
	if (m_width <= 0 || m_height <= 0) return;
	if (m_tileSize.x <= 0 || m_tileSize.y <= 0) return;

	// If tiles array not provided, fill with empty.
	const int expected = m_width * m_height;
	if ((int)m_tiles.size() != expected) {
		m_tiles.assign(expected, -1);
	}

	int tileId = 0;
	m_spawned.reserve((size_t)expected);

	for (int y = 0; y < m_height; ++y) {
		for (int x = 0; x < m_width; ++x) {
			const int idx = Index(x, y);
			const int frameIndex = m_tiles[idx];
			if (frameIndex < 0) continue;

			auto tile = scene->CreateGameObject<GameObject>("Tile_" + std::to_string(tileId++));
			tile->GetTransform()->SetParent(parent);
			tile->GetTransform()->SetPosition(TileLocalCenter(x, y));

			auto sr = tile->AddComponent<SpriteRenderer>();
			sr->SetTexture(m_texture);
			sr->SetFrameSize(m_tileSize);
			sr->SetFrameIndex(frameIndex);
			sr->SetLayerOrder(m_layerOrder);

			m_spawned.push_back(tile);
		}
	}
}

#pragma once

#include <memory>
#include <vector>

#include "MonoBehaviour.h"
#include "Types.hpp"

class Texture;
class GameObject;
class Transform;

// Simple sprited tilemap that builds a grid out of SpriteRenderer children.
//
// - Each tile is a frame index inside a tileset texture.
// - The map is laid out in local space under the owning GameObject.
// - Coordinate convention: (0,0) is the TOP-LEFT tile of the map.
//
// This is intentionally lightweight for course projects: it trades runtime efficiency
// for simplicity and clarity.
class Tilemap : public MonoBehaviour {
public:
	Tilemap();

	// Tileset texture + tile size (frame size) + how many tiles per row in the tileset.
	void SetTileset(Texture* texture, const Vector2i& tileSize, int tilesPerRow);

	// Map size in tiles.
	void SetMapSize(int width, int height);

	// Tile indices (size must be width*height). Use -1 for empty.
	void SetTiles(const std::vector<int>& tiles);

	// Layer order applied to each tile SpriteRenderer.
	void SetLayerOrder(int order) { m_layerOrder = order; }

	// If true, builds in Start(). Default: true.
	void SetBuildOnStart(bool v) { m_buildOnStart = v; }

	// Rebuild tile GameObjects.
	void Rebuild();

	// Destroy spawned tile GameObjects.
	void Clear();

protected:
	void Start() override;

private:
	void BuildInternal();
	void DestroySpawned();
	int Index(int x, int y) const { return y * m_width + x; }
	Vector2f TileLocalCenter(int x, int y) const;

	Texture* m_texture = nullptr;
	Vector2i m_tileSize = Vector2i(32, 32);
	int m_tilesPerRow = 16;

	int m_width = 0;
	int m_height = 0;
	std::vector<int> m_tiles;

	int m_layerOrder = -20;
	bool m_buildOnStart = true;

	std::vector<std::weak_ptr<GameObject>> m_spawned;
};

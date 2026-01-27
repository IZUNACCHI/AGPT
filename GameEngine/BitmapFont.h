#pragma once

#include "Types.hpp"
#include <array>
#include <string>
#include <vector>

class Texture;
class Renderer;

// Fixed-grid font + explicit mapping support (no ASCII ordering required)
class BitmapFont {
public:
	// Default mapping: sequential starting at firstChar (only useful if the sheet is ordered).
	BitmapFont(Texture* texture, const Vector2i& glyphSize, unsigned char firstChar = 32);

	void SetSpacing(const Vector2i& spacing) { m_spacing = spacing; }
	Vector2i GetSpacing() const { return m_spacing; }

	Texture* GetTexture() const { return m_texture; }
	Vector2i GetGlyphSize() const { return m_glyphSize; }

	// rows must match grid exactly. '_' => space, '.' => unused
	bool SetLayoutRows(const std::vector<std::string>& rows, char spacePlaceholder = '_', char emptyPlaceholder = '.');

	bool MapCharToCell(char c, int col, int row);

	bool GetGlyphSourceRect(char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const;

	// Unrotated draw (TextRenderer will do rotation by drawing glyphs rotated).
	void Draw(Renderer& renderer, const std::string& text, const Vector2f& worldTopLeft, const Vector2f& scale) const;

	Vector2f MeasureText(const std::string& text, const Vector2f& scale) const;

private:
	bool GetSourceForChar(unsigned char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const;

private:
	Texture* m_texture = nullptr; // not owned
	Vector2i m_glyphSize{ 16, 16 };
	Vector2i m_spacing{ 0, 0 };

	int m_columns = 0;
	int m_rows = 0;
	int m_glyphCount = 0;
	unsigned char m_firstChar = 32;

	std::array<int, 256> m_charToIndex{};
};

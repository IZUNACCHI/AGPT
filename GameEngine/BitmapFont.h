#pragma once

#include "Types.hpp"
#include <array>
#include <string>
#include <vector>

class Texture;

// Fixed-grid bitmap font with explicit mapping:
//
// Mapping options:
// 1) Default sequential mapping (firstChar..)
// 2) SetLayoutRows(...) to explicitly define what each cell is
//    - '_' means space
//    - '.' means unused
class BitmapFont {
public:
	BitmapFont(Texture* texture, const Vector2i& glyphSize, unsigned char firstChar = 32);

	void SetSpacing(const Vector2i& spacing) { m_spacing = spacing; }
	Vector2i GetSpacing() const { return m_spacing; }

	Texture* GetTexture() const { return m_texture; }
	Vector2i GetGlyphSize() const { return m_glyphSize; }

	int GetColumns() const { return m_columns; }
	int GetRows() const { return m_rows; }

	// Explicit layout: rows.size must match grid rows, and each string length must match grid columns.
	// spacePlaceholder '_' -> ' '
	// emptyPlaceholder '.' -> ignored
	bool SetLayoutRows(const std::vector<std::string>& rows, char spacePlaceholder = '_', char emptyPlaceholder = '.');

	// Direct mapping: map a character to a grid cell.
	bool MapCharToCell(char c, int col, int row);

	// Get glyph source rect in texture for a character.
	// Returns false only if the font is invalid (no texture/grid).
	bool GetGlyphSource(char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const;

	// Measures the text block size in "font units" (pixels) BEFORE any extra scale is applied.
	// (So you can apply non-uniform scale yourself.)
	Vector2f MeasureTextUnscaled(const std::string& text) const;

private:
	bool IsValidGrid() const;

	int ResolveIndex(unsigned char c) const;

private:
	Texture* m_texture = nullptr; // not owned
	Vector2i m_glyphSize{ 16, 16 };
	Vector2i m_spacing{ 0, 0 };

	int m_columns = 0;
	int m_rows = 0;
	int m_glyphCount = 0;

	unsigned char m_firstChar = 32;

	// char -> glyphIndex (0..glyphCount-1), -1 = unmapped
	std::array<int, 256> m_charToIndex{};
};

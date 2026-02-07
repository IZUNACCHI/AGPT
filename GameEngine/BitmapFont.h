#pragma once

#include "Types.hpp"
#include <array>
#include <string>
#include <vector>

class Texture;
class Renderer;

// BitmapFont: fixed-size glyphs in a texture atlas, mapped to ASCII chars. Can be mapped with string table
class BitmapFont {
public:
	// Default mapping: sequential starting at firstChar (only useful if the sheet is ordered lke ASCII).
	BitmapFont(Texture* texture, const Vector2i& glyphSize, unsigned char firstChar = 32);

	// Spacing between glyphs in the texture atlas.
	void SetSpacing(const Vector2i& spacing) { m_spacing = spacing; }
	Vector2i GetSpacing() const { return m_spacing; }

	Texture* GetTexture() const { return m_texture; }
	Vector2i GetGlyphSize() const { return m_glyphSize; }

	// rows must match grid exactly. '_' = space, '.' = unused
	bool SetLayoutRows(const std::vector<std::string>& rows, char spacePlaceholder = '_', char emptyPlaceholder = '.');

	// Map a character to a specific cell in the grid.
	bool MapCharToCell(char c, int col, int row);

	// Get the source rectangle in the texture for the given character.
	bool GetGlyphSourceRect(char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const;

	// Unrotated draw (TextRenderer will do rotation by drawing glyphs rotated).
	void Draw(Renderer& renderer, const std::string& text, const Vector2f& worldTopLeft, const Vector2f& scale) const;
	void DrawColored(Renderer& renderer, const std::string& text, const Vector2f& worldTopLeft, const Vector2f& scale, const Vector4i& color) const;

	// Measure the size of the given text string when rendered with this font at the given scale.
	Vector2f MeasureText(const std::string& text, const Vector2f& scale) const;

private:
	bool GetSourceForChar(unsigned char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const;

private:
	Texture* m_texture = nullptr; // cached texture atlas
	Vector2i m_glyphSize{ 16, 16 }; // width, height in pixels
	Vector2i m_spacing{ 0, 0 }; // spacing between glyphs in atlas

	int m_columns = 0; // number of columns in the atlas
	int m_rows = 0; // number of rows in the atlas
	int m_glyphCount = 0; // total number of glyphs in the atlas
	unsigned char m_firstChar = 32; // ASCII code of the first glyph

	std::array<int, 256> m_charToIndex{}; // mapping from char code to glyph index in the atlas
};

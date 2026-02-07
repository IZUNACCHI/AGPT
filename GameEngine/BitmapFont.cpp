#include "BitmapFont.h"
#include "Texture.h"
#include "Renderer.h"
#include <cmath>

BitmapFont::BitmapFont(Texture* texture, const Vector2i& glyphSize, unsigned char firstChar)
	: m_texture(texture), m_glyphSize(glyphSize), m_firstChar(firstChar) {

	m_charToIndex.fill(-1);

	if (!m_texture || !m_texture->IsValid() || m_glyphSize.x <= 0 || m_glyphSize.y <= 0) {
		return;
	}

	const Vector2i texSize = m_texture->GetSize();
	m_columns = texSize.x / m_glyphSize.x;
	m_rows = texSize.y / m_glyphSize.y;
	m_glyphCount = m_columns * m_rows;

	// Default sequential mapping
	for (int i = 0; i < m_glyphCount; ++i) {
		const int code = (int)m_firstChar + i;
		if (code >= 0 && code < 256) {
			m_charToIndex[(unsigned char)code] = i;
		}
	}
}

bool BitmapFont::SetLayoutRows(const std::vector<std::string>& rows, char spacePlaceholder, char emptyPlaceholder) {
	if (m_columns <= 0 || m_rows <= 0 || m_glyphCount <= 0) return false;
	if ((int)rows.size() != m_rows) return false;

	m_charToIndex.fill(-1);

	for (int r = 0; r < m_rows; ++r) {
		if ((int)rows[r].size() != m_columns) return false;

		for (int c = 0; c < m_columns; ++c) {
			char ch = rows[r][c];
			if (ch == emptyPlaceholder) continue;
			if (ch == spacePlaceholder) ch = ' ';

			const int index = r * m_columns + c;
			m_charToIndex[(unsigned char)ch] = index;
		}
	}
	return true;
}

bool BitmapFont::MapCharToCell(char c, int col, int row) {
	if (col < 0 || row < 0 || col >= m_columns || row >= m_rows) return false;
	const int index = row * m_columns + col;
	if (index < 0 || index >= m_glyphCount) return false;
	m_charToIndex[(unsigned char)c] = index;
	return true;
}

bool BitmapFont::GetSourceForChar(unsigned char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const {
	if (!m_texture || !m_texture->IsValid() || m_columns <= 0 || m_rows <= 0 || m_glyphCount <= 0) return false;

	int index = m_charToIndex[c];

	// Fallback to '?' then 0
	if (index < 0 || index >= m_glyphCount) {
		index = m_charToIndex[(unsigned char)'?'];
		if (index < 0 || index >= m_glyphCount) index = 0;
	}

	const int col = index % m_columns;
	const int row = index / m_columns;

	outSrcPos = Vector2f((float)(col * m_glyphSize.x), (float)(row * m_glyphSize.y));
	outSrcSize = Vector2f((float)m_glyphSize.x, (float)m_glyphSize.y);
	return true;
}

bool BitmapFont::GetGlyphSourceRect(char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const {
	return GetSourceForChar((unsigned char)c, outSrcPos, outSrcSize);
}

void BitmapFont::Draw(Renderer& renderer, const std::string& text, const Vector2f& worldTopLeft, const Vector2f& scale) const {
	if (!m_texture || !m_texture->IsValid()) return;

	const float sx = std::abs(scale.x);
	const float sy = std::abs(scale.y);

	const float advX = (float)(m_glyphSize.x + m_spacing.x) * sx;
	const float advY = (float)(m_glyphSize.y + m_spacing.y) * sy;

	Vector2f pen = worldTopLeft;

	for (char ch : text) {
		if (ch == '\n') {
			pen.x = worldTopLeft.x;
			pen.y -= advY; // down visually (+Y up world)
			continue;
		}

		Vector2f srcPos, srcSize;
		if (!GetSourceForChar((unsigned char)ch, srcPos, srcSize)) return;

		const Vector2f dstSize(srcSize.x * sx, srcSize.y * sy);
		renderer.DrawTexture(*m_texture, srcPos, srcSize, pen, dstSize);

		pen.x += advX;
	}
}

void BitmapFont::DrawColored(Renderer& renderer, const std::string& text, const Vector2f& worldTopLeft, const Vector2f& scale, const Vector4i& color) const {
	if (!m_texture || !m_texture->IsValid()) return;

	const float sx = std::abs(scale.x);
	const float sy = std::abs(scale.y);

	const float advX = (float)(m_glyphSize.x + m_spacing.x) * sx;
	const float advY = (float)(m_glyphSize.y + m_spacing.y) * sy;

	Vector2f pen = worldTopLeft;

	for (char ch : text) {
		if (ch == '\n') {
			pen.x = worldTopLeft.x;
			pen.y -= advY;
			continue;
		}

		Vector2f srcPos, srcSize;
		if (!GetSourceForChar((unsigned char)ch, srcPos, srcSize)) return;

		const Vector2f dstSize(srcSize.x * sx, srcSize.y * sy);
		renderer.DrawTextureTinted(*m_texture, srcPos, srcSize, pen, dstSize, color);

		pen.x += advX;
	}
}

Vector2f BitmapFont::MeasureText(const std::string& text, const Vector2f& scale) const {
	if (!m_texture || !m_texture->IsValid()) return Vector2f::Zero();

	const float sx = std::abs(scale.x);
	const float sy = std::abs(scale.y);

	const float advX = (float)(m_glyphSize.x + m_spacing.x) * sx;
	const float lineH = (float)m_glyphSize.y * sy;
	const float gapY = (float)m_spacing.y * sy;

	float maxW = 0.0f;
	float lineW = 0.0f;
	int lines = 1;

	for (char ch : text) {
		if (ch == '\n') {
			if (lineW > maxW) maxW = lineW;
			lineW = 0.0f;
			lines++;
		}
		else {
			lineW += advX;
		}
	}
	if (lineW > maxW) maxW = lineW;

	const float totalH = (float)lines * lineH + (float)(lines - 1) * gapY;
	return Vector2f(maxW, totalH);
}

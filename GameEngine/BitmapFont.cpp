#include "BitmapFont.h"
#include "Texture.h"

BitmapFont::BitmapFont(Texture* texture, const Vector2i& glyphSize, unsigned char firstChar)
	: m_texture(texture), m_glyphSize(glyphSize), m_firstChar(firstChar) {

	m_charToIndex.fill(-1);

	if (!m_texture || !m_texture->IsValid() || m_glyphSize.x <= 0 || m_glyphSize.y <= 0) {
		m_columns = m_rows = m_glyphCount = 0;
		return;
	}

	const Vector2i texSize = m_texture->GetSize();
	m_columns = texSize.x / m_glyphSize.x;
	m_rows = texSize.y / m_glyphSize.y;
	m_glyphCount = m_columns * m_rows;

	// Default sequential mapping
	for (int i = 0; i < m_glyphCount; ++i) {
		const int code = static_cast<int>(m_firstChar) + i;
		if (code >= 0 && code < 256) {
			m_charToIndex[static_cast<unsigned char>(code)] = i;
		}
	}
}

bool BitmapFont::IsValidGrid() const {
	return m_texture && m_texture->IsValid() && m_columns > 0 && m_rows > 0 && m_glyphCount > 0;
}

bool BitmapFont::SetLayoutRows(const std::vector<std::string>& rows, char spacePlaceholder, char emptyPlaceholder) {
	if (!IsValidGrid()) return false;
	if (static_cast<int>(rows.size()) != m_rows) return false;

	m_charToIndex.fill(-1);

	for (int r = 0; r < m_rows; ++r) {
		if (static_cast<int>(rows[r].size()) != m_columns) return false;

		for (int c = 0; c < m_columns; ++c) {
			char ch = rows[r][c];
			if (ch == emptyPlaceholder) continue;
			if (ch == spacePlaceholder) ch = ' ';

			const int index = r * m_columns + c;
			m_charToIndex[static_cast<unsigned char>(ch)] = index;
		}
	}
	return true;
}

bool BitmapFont::MapCharToCell(char c, int col, int row) {
	if (!IsValidGrid()) return false;
	if (col < 0 || row < 0 || col >= m_columns || row >= m_rows) return false;

	const int index = row * m_columns + col;
	if (index < 0 || index >= m_glyphCount) return false;

	m_charToIndex[static_cast<unsigned char>(c)] = index;
	return true;
}

int BitmapFont::ResolveIndex(unsigned char c) const {
	int index = m_charToIndex[c];

	// fallback to '?', then 0
	if (index < 0 || index >= m_glyphCount) {
		index = m_charToIndex[static_cast<unsigned char>('?')];
		if (index < 0 || index >= m_glyphCount) index = 0;
	}
	return index;
}

bool BitmapFont::GetGlyphSource(char c, Vector2f& outSrcPos, Vector2f& outSrcSize) const {
	if (!IsValidGrid()) return false;

	const int index = ResolveIndex(static_cast<unsigned char>(c));
	const int col = index % m_columns;
	const int row = index / m_columns;

	outSrcPos = Vector2f(
		static_cast<float>(col * m_glyphSize.x),
		static_cast<float>(row * m_glyphSize.y)
	);

	outSrcSize = Vector2f(
		static_cast<float>(m_glyphSize.x),
		static_cast<float>(m_glyphSize.y)
	);

	return true;
}

Vector2f BitmapFont::MeasureTextUnscaled(const std::string& text) const {
	if (!IsValidGrid()) return Vector2f(0.0f, 0.0f);

	const float advX = static_cast<float>(m_glyphSize.x + m_spacing.x);
	const float lineH = static_cast<float>(m_glyphSize.y);
	const float gapY = static_cast<float>(m_spacing.y);

	float maxW = 0.0f;
	float lineW = 0.0f;
	int lines = 1;

	for (char ch : text) {
		if (ch == '\n') {
			if (lineW > maxW) maxW = lineW;
			lineW = 0.0f;
			lines++;
			continue;
		}
		lineW += advX;
	}
	if (lineW > maxW) maxW = lineW;

	const float totalH = lines * lineH + (lines - 1) * gapY;
	return Vector2f(maxW, totalH);
}

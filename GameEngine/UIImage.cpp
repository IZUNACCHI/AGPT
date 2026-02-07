#include "UIImage.h"

#include "Renderer.h"
#include "Texture.h"

UIImage::UIImage() : UIElement() {
}

void UIImage::Render(Renderer& renderer) {
	if (!m_texture) return;

	Vector2f worldTopLeft = UIToWorldTopLeft(Vector2f(m_rect.x, m_rect.y));
	Vector2f dstSize(m_rect.w, m_rect.h);

	Vector2f srcPos(0.0f, 0.0f);
	Vector2f srcSize;
	if (m_hasSource) {
		srcPos = m_srcPos;
		srcSize = m_srcSize;
	} else {
		Vector2i s = m_texture->GetSize();
		srcSize = Vector2f((float)s.x, (float)s.y);
	}

	renderer.DrawTexture(*m_texture, srcPos, srcSize, worldTopLeft, dstSize);
}

#include "AnimationClip.h"

void AnimationClip::AddFrame(int row, int col, int frameWidth, int frameHeight, float duration) {
	Rect src;
	src.x = col * frameWidth;
	src.y = row * frameHeight;
	src.w = frameWidth;
	src.h = frameHeight;

	m_frames.push_back({ src, duration });
}
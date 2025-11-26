#pragma once
#include <vector>
#include <string>
#include "Rect.hpp"

struct AnimationFrame {
	Rect srcRect;   // portion of the spritesheet
	float duration; // seconds this frame is shown
};

class AnimationClip {
public:
	AnimationClip(const std::string& name = "NewClip") : m_name(name), m_loop(true), m_speed(1.0f) {
	}

	// Manually add a frame by row/col
	void AddFrame(int row, int col, int frameWidth, int frameHeight, float duration = 0.1f);

	// Clear all frames
	void ClearFrames() { m_frames.clear(); }

	const std::string& GetName() const { return m_name; }
	void SetLoop(bool loop) { m_loop = loop; }
	void SetSpeed(float speed) { m_speed = speed; } // 1.0 = normal

	const std::vector<AnimationFrame>& GetFrames() const { return m_frames; }
	bool IsLooping() const { return m_loop; }
	float GetSpeed() const { return m_speed; }

private:
	std::string m_name;
	std::vector<AnimationFrame> m_frames;
	bool m_loop;
	float m_speed;
};
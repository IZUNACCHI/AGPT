#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#include "SpriteSheet.h"

/// Sprite-only animation clip.
/// A clip is a sequence of spritesheet frame indices played at a fixed FPS.
class AnimationClip {
public:
	struct Event {
		/// Fired when playback enters this local frame index (0..frames.size-1).
		int localFrameIndex = 0;
		std::string name;
	};

	std::string name;

	/// Which spritesheet to use for this clip.
	SpriteSheet* sheet = nullptr;

	/// Spritesheet frame indices (row-major) in playback order.
	std::vector<int> frames;

	float fps = 12.0f;
	bool loop = true;

	std::vector<Event> events;

	bool IsValid() const {
		return sheet != nullptr && sheet->IsValid() && !frames.empty() && fps > 0.0f;
	}

	float GetLengthSeconds() const {
		if (fps <= 0.0f || frames.empty()) return 0.0f;
		return static_cast<float>(frames.size()) / fps;
	}

	/// Sample the spritesheet frameIndex to display at time t (seconds since clip start).
	/// Returns -1 if invalid.
	int SampleFrameIndex(float t) const {
		int local = SampleLocalFrame(t);
		if (local < 0) return -1;
		return frames[static_cast<size_t>(local)];
	}

	/// Sample the local frame cursor (0..frames.size-1) at time t.
	/// Returns -1 if invalid.
	int SampleLocalFrame(float t) const {
		if (frames.empty() || fps <= 0.0f) return -1;

		int local = static_cast<int>(std::floor(t * fps));
		if (loop) {
			const int n = static_cast<int>(frames.size());
			local = ((local % n) + n) % n;
		} else {
			local = std::max(0, local);
			local = std::min(local, static_cast<int>(frames.size()) - 1);
		}
		return local;
	}

	/// Normalized time in [0..1].
	/// Looping clips return the fractional progress within the current loop.
	float GetNormalizedTime(float t) const {
		const float len = GetLengthSeconds();
		if (len <= 0.0f) return 0.0f;

		if (loop) {
			float n = t / len;
			float frac = n - std::floor(n);
			if (frac < 0.0f) frac += 1.0f;
			return frac;
		}

		float n = t / len;
		if (n < 0.0f) n = 0.0f;
		if (n > 1.0f) n = 1.0f;
		return n;
	}
};

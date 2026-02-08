#pragma once

#include <GameEngine/GameEngine.h>
#include <GameEngine/Animator.h>

#include <algorithm>

// Build a simple 1-state looping controller that plays every frame in the provided sheet.
//
// NOTE:
// - The AnimationClip and AnimatorController are stored by reference in the Animator.
//   Keep the clip+controller as members of your behaviour, not as locals.
inline void BuildLoopAllFrames(SpriteSheet* sheet, float fps, AnimationClip& outClip, AnimatorController& outCtrl) {
	if (!sheet || !sheet->IsValid()) {
		THROW_ENGINE_EXCEPTION("BuildLoopAllFrames: invalid spritesheet");
	}

	if (fps <= 0.0f) fps = 12.0f;

	outClip = AnimationClip{};
	outClip.name = "Loop";
	outClip.sheet = sheet;
	outClip.fps = fps;
	outClip.loop = true;
	outClip.frames.clear();

	const Vector2i texSize = sheet->texture->GetSize();
	const int cols = (sheet->frameSize.x > 0) ? (texSize.x / sheet->frameSize.x) : 0;
	const int rows = (sheet->frameSize.y > 0) ? (texSize.y / sheet->frameSize.y) : 0;
	const int total = std::max(0, cols * rows);
	for (int i = 0; i < total; ++i) {
		outClip.frames.push_back(i);
	}
	if (outClip.frames.empty()) {
		outClip.frames.push_back(0);
	}

	outCtrl = AnimatorController{};
	AnimState st;
	st.id = 0;
	st.name = "Loop";
	st.clip = &outClip;
	outCtrl.states = { st };
	outCtrl.transitions.clear();
	outCtrl.parameters.clear();
	outCtrl.entryState = 0;
}

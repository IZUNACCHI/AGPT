#pragma once

#include "ProjectileCommon.hpp"
#include "../GameEngine/Animator.h"
#include "XenonAssetKeys.h"

// -----------------------------------------------------------------------------
// Enemy projectile (EnWeap6.bmp)
// - Loops the entire spritesheet using Animator
// -----------------------------------------------------------------------------

namespace EnemyProjAnim {
	inline SpriteSheet* GetSheet() {
		return LoadSpriteSheet(XenonAssetKeys::Sheets::EnemyProjectiles, "EnWeap6.bmp", Vector2i(16, 16), Vector3i(255, 0, 255));
	}

	// Shared templates (clip/controller). Runtime state is per-entity Animator.
	inline bool g_built = false;
	inline AnimationClip g_clip{};
	inline AnimatorController g_ctrl{};

	inline void BuildOnce() {
		if (g_built) return;

		SpriteSheet* sheet = GetSheet();
		if (!sheet || !sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load enemy projectile spritesheet (EnWeap6.bmp)");
		}

		const Vector2i texSize = sheet->texture->GetSize();
		const int cols = sheet->frameSize.x > 0 ? (texSize.x / sheet->frameSize.x) : 0;
		const int rows = sheet->frameSize.y > 0 ? (texSize.y / sheet->frameSize.y) : 0;
		const int total = std::max(0, cols * rows);
		if (total <= 0) {
			THROW_ENGINE_EXCEPTION("Enemy projectile spritesheet has 0 frames");
		}

		g_clip = AnimationClip{};
		g_clip.name = "Loop";
		g_clip.sheet = sheet;
		g_clip.fps = 16.0f;
		g_clip.loop = true;
		g_clip.frames.clear();
		for (int i = 0; i < total; ++i) {
			g_clip.frames.push_back(i);
		}

		g_ctrl = AnimatorController{};
		AnimState st;
		st.id = 0;
		st.name = "Loop";
		st.clip = &g_clip;
		g_ctrl.states = { st };
		g_ctrl.transitions.clear();
		g_ctrl.parameters.clear();
		g_ctrl.entryState = 0;

		g_built = true;
	}

	inline AnimatorController* GetController(AnimationClip*& outClipPtr) {
		BuildOnce();
		outClipPtr = &g_clip;
		return &g_ctrl;
	}
}

class EnemyProjectileSheetBehaviour : public EnemyProjectileBehaviour {
public:
	EnemyProjectileSheetBehaviour() {
		SetComponentName("EnemyProjectileSheetBehaviour");
	}

protected:
	void Awake() override {
		EnemyProjectileBehaviour::Awake();

		animator = GetComponent<Animator>().get();
		if (!animator) {
			THROW_ENGINE_EXCEPTION("Enemy projectile is missing Animator component");
		}

		AnimationClip* clip = nullptr;
		AnimatorController* ctrl = EnemyProjAnim::GetController(clip);
		if (!ctrl || !clip) {
			THROW_ENGINE_EXCEPTION("Failed to build enemy projectile animator controller");
		}
		animator->SetController(ctrl);
		animator->Play("Loop", true);
	}

	void ConfigureSprite() override {
		if (!sprite) return;
		SpriteSheet* sheet = EnemyProjAnim::GetSheet();
		if (!sheet || !sheet->IsValid()) return;
		sprite->SetTexture(sheet->texture);
		sprite->SetFrameSize(sheet->frameSize);
		sprite->SetFrameIndex(0);
		sprite->SetLayerOrder(1);
	}

private:
	Animator* animator = nullptr;
};

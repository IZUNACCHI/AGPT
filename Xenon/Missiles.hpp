#pragma once

#include "ProjectileCommon.hpp"
#include <GameEngine/Animator.h>
#include "XenonAssetKeys.h"



enum class MissileType {
	Light = 0,
	Medium = 1,
	Heavy = 2,
};

inline int MissileDamage(MissileType type) {
	switch (type) {
	case MissileType::Light: return 1;
	case MissileType::Medium: return 2;
	case MissileType::Heavy: return 4;
	default: return 1;
	}
}

namespace MissileAnim {
	inline SpriteSheet* GetSheet() {
		// Let AssetManager own the cache. No header-local static pointers.
		return LoadSpriteSheet(XenonAssetKeys::Sheets::Missiles, "missile.bmp", Vector2i(16, 16), Vector3i(255, 0, 255));
	}

	inline void BuildLoopForRow(MissileType type, AnimationClip& outClip, AnimatorController& outCtrl) {
		SpriteSheet* sheet = GetSheet();
		if (!sheet || !sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load missile spritesheet (missile.bmp)");
		}

		const Vector2i texSize = sheet->texture->GetSize();
		const int cols = sheet->frameSize.x > 0 ? (texSize.x / sheet->frameSize.x) : 0;
		if (cols <= 0) {
			THROW_ENGINE_EXCEPTION("Missile spritesheet has invalid column count");
		}

		outClip = AnimationClip{};
		outClip.name = "Loop";
		outClip.sheet = sheet;
		outClip.fps = 16.0f;
		outClip.loop = true;
		outClip.frames.clear();

		const int row = static_cast<int>(type);
		const int start = row * cols;
		for (int i = 0; i < cols; ++i) {
			outClip.frames.push_back(start + i);
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

	// One controller+clip per missile type (shared templates). C++17 inline variables
	// keep these singletons across translation units.
	inline bool g_built[3] = { false, false, false };
	inline AnimationClip g_clips[3]{};
	inline AnimatorController g_ctrls[3]{};

	inline AnimatorController* GetController(MissileType type, AnimationClip*& outClipPtr) {
		// One controller+clip per missile type (shared templates).
		// Runtime state is per Animator component instance.

		const int idx = static_cast<int>(type);
		if (idx < 0 || idx > 2) {
			outClipPtr = nullptr;
			return nullptr;
		}

		if (!g_built[idx]) {
			BuildLoopForRow(type, g_clips[idx], g_ctrls[idx]);
			g_built[idx] = true;
		}

		outClipPtr = &g_clips[idx];
		return &g_ctrls[idx];
	}
}

class MissileBehaviour : public ProjectileBehaviour {
public:
	MissileBehaviour()
		: ProjectileBehaviour("MissileBehaviour") {
		SetFaction(Faction::Player);
		SetMissileType(MissileType::Light);
	}

	void SetMissileType(MissileType type) {
		m_type = type;
		SetDamage(MissileDamage(type));
	}
	MissileType GetMissileType() const { return m_type; }

protected:
	void Awake() override {
		// Make sure sprite/rigidbody/collider exist and collider gets sized.
		ProjectileBehaviour::Awake();

		animator = GetComponent<Animator>().get();
		if (!animator) {
			THROW_ENGINE_EXCEPTION("Missile is missing Animator component");
		}

		// Hook Animator graph.
		AnimationClip* clip = nullptr;
		AnimatorController* ctrl = MissileAnim::GetController(m_type, clip);
		if (!ctrl || !clip) {
			THROW_ENGINE_EXCEPTION("Failed to build missile animator controller");
		}
		animator->SetController(ctrl);
		animator->Play("Loop", true);
	}

	void ConfigureSprite() override {
		// Requirement: show correct texture immediately on creation.
		if (!sprite) return;
		SpriteSheet* sheet = MissileAnim::GetSheet();
		if (!sheet || !sheet->IsValid()) return;
		sprite->SetTexture(sheet->texture);
		sprite->SetFrameSize(sheet->frameSize);
		sprite->SetFrameIndex(0);
		sprite->SetLayerOrder(1);
	}

private:
	MissileType m_type = MissileType::Light;
	Animator* animator = nullptr;
};

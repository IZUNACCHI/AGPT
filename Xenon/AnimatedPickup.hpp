#pragma once

#include "Pickup.hpp"

#include "../GameEngine/Animator.h"

#include <algorithm>

// -----------------------------------------------------------------------------
// AnimatedPickup
//
// A Pickup variant that includes an Animator and a helper behaviour that builds
// a simple looping animation from an entire spritesheet.
//
// Intended use:
// - Derive your pickup behaviour from AnimatedPickupBehaviour.
// - In the constructor, configure the spritesheet path, frame size, FPS, etc.
// - Derive your pickup GameObject from AnimatedPickup (instead of Pickup).
// -----------------------------------------------------------------------------

class AnimatedPickupBehaviour : public PickupBehaviour {
public:
	AnimatedPickupBehaviour() : PickupBehaviour() {
		SetComponentName("AnimatedPickupBehaviour");
	}

	// Visual configuration (call these in your derived behaviour's constructor).
	void SetSpriteSheetPath(const std::string& path) { m_sheetPath = path; }
	void SetFrameSize(const Vector2i& size) { m_frameSize = size; }
	void SetFPS(float fps) { m_fps = fps; }
	void SetLayerOrder(int order) { m_layerOrder = order; }
	void SetColorKey(const Vector3i& colorKey) { m_useColorKey = true; m_colorKey = colorKey; }
	void ClearColorKey() { m_useColorKey = false; }

protected:
	void Awake() override {
		// Configure sprite first so PickupBehaviour can size the collider correctly.
		auto spr = GetComponent<SpriteRenderer>().get();
		if (!spr) {
			THROW_ENGINE_EXCEPTION("AnimatedPickup is missing SpriteRenderer component");
		}

		animator = GetComponent<Animator>().get();
		if (!animator) {
			THROW_ENGINE_EXCEPTION("AnimatedPickup is missing Animator component");
		}

		BuildAssetsIfNeeded();

		// Requirement: set texture immediately when created.
		if (m_sheet && m_sheet->texture) {
			spr->SetTexture(m_sheet->texture);
			spr->SetFrameSize(m_frameSize);
			spr->SetFrameIndex(0);
		}
		spr->SetLayerOrder(m_layerOrder);

		// Now let PickupBehaviour do its setup (rigidbody/collider/trigger + collider sizing).
		PickupBehaviour::Awake();

		// Start looping.
		animator->SetController(&m_controller);
		animator->Play("Loop", true);
	}

	// You can override this if you want a custom frame list (not all frames).
	virtual void BuildFrames(std::vector<int>& outFrames) {
		outFrames.clear();

		if (!m_sheet || !m_sheet->IsValid()) return;
		const Vector2i texSize = m_sheet->texture->GetSize();
		const int cols = (m_frameSize.x > 0) ? (texSize.x / m_frameSize.x) : 0;
		const int rows = (m_frameSize.y > 0) ? (texSize.y / m_frameSize.y) : 0;
		const int total = std::max(0, cols * rows);
		for (int i = 0; i < total; ++i) {
			outFrames.push_back(i);
		}
	}

protected:
	Animator* animator = nullptr;

private:
	static std::string BuildPickupSheetKey(const std::string& path, const Vector2i& frame, bool useColorKey, const Vector3i& ck) {
		// Stable-ish key for caching in AssetManager.
		std::string k = "sheet.pickup.";
		k += path;
		k += "|" + std::to_string(frame.x) + "x" + std::to_string(frame.y);
		if (useColorKey) {
			k += "|ck=" + std::to_string(ck.x) + "," + std::to_string(ck.y) + "," + std::to_string(ck.z);
		}
		return k;
	}

	void BuildAssetsIfNeeded() {
		if (m_built) return;

		if (m_sheetPath.empty()) {
			THROW_ENGINE_EXCEPTION("AnimatedPickupBehaviour has no spritesheet path configured");
		}
		if (m_frameSize.x <= 0 || m_frameSize.y <= 0) {
			THROW_ENGINE_EXCEPTION("AnimatedPickupBehaviour has an invalid frame size");
		}
		if (m_fps <= 0.0f) {
			m_fps = 12.0f;
		}

		const std::string sheetKey = BuildPickupSheetKey(m_sheetPath, m_frameSize, m_useColorKey, m_colorKey);
		if (m_useColorKey) {
			m_sheet = LoadSpriteSheet(sheetKey, m_sheetPath, m_frameSize, m_colorKey);
		} else {
			m_sheet = LoadSpriteSheet(sheetKey, m_sheetPath, m_frameSize);
		}
		if (!m_sheet || !m_sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load pickup spritesheet: " + m_sheetPath);
		}

		// Build a looping clip.
		m_clip = AnimationClip{};
		m_clip.name = "Loop";
		m_clip.sheet = m_sheet;
		m_clip.fps = m_fps;
		m_clip.loop = true;
		BuildFrames(m_clip.frames);
		if (m_clip.frames.empty()) {
			// Fallback: show frame 0.
			m_clip.frames.push_back(0);
		}

		// Build a controller with a single looping state.
		m_controller = AnimatorController{};
		AnimState st;
		st.id = 0;
		st.name = "Loop";
		st.clip = &m_clip;
		m_controller.states = { st };
		m_controller.transitions.clear();
		m_controller.parameters.clear();
		m_controller.entryState = 0;

		m_built = true;
	}

private:
	std::string m_sheetPath;
	Vector2i m_frameSize = Vector2i(32, 32);
	float m_fps = 12.0f;
	int m_layerOrder = 2;

	bool m_useColorKey = false;
	Vector3i m_colorKey = Vector3i(255, 0, 255);

	bool m_built = false;
	SpriteSheet* m_sheet = nullptr;
	AnimationClip m_clip{};
	AnimatorController m_controller{};
};

// GameObject base that includes Animator.
class AnimatedPickup : public Pickup {
public:
	explicit AnimatedPickup(const std::string& name = "AnimatedPickup")
		: Pickup(name) {
		AddComponent<Animator>();
		// NOTE: Behaviour is added by subclasses.
	}
};

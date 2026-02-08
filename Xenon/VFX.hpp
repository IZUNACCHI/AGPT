#pragma once

#include <GameEngine/GameEngine.h>
#include <GameEngine/Animator.h>


class OneShotVFXBehaviour : public MonoBehaviour {
public:
	OneShotVFXBehaviour() {
		SetComponentName("OneShotVFXBehaviour");
	}

	// Configure what to play.
	void SetSpriteSheetPath(const std::string& path) { m_sheetPath = path; }
	void SetFrameSize(const Vector2i& size) { m_frameSize = size; }
	void SetFPS(float fps) { m_fps = fps; }
	void SetColorKey(const Vector3i& colorKey) { m_useColorKey = true; m_colorKey = colorKey; }

protected:
	void Awake() override {
		sprite = GetComponent<SpriteRenderer>().get();
		animator = GetComponent<Animator>().get();

		if (!sprite) {
			THROW_ENGINE_EXCEPTION("VFX " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ") is missing SpriteRenderer");
		}
		if (!animator) {
			THROW_ENGINE_EXCEPTION("VFX " + GetGameObject()->GetName() + " (" + std::to_string(GetGameObject()->GetInstanceID()) + ") is missing Animator");
		}

		BuildAssetsIfNeeded();

		// Requirement: set texture immediately when created.
		if (m_sheet && m_sheet->texture) {
			sprite->SetTexture(m_sheet->texture);
			sprite->SetFrameSize(m_frameSize);
			sprite->SetFrameIndex(0);
		}

		animator->SetController(&m_controller);
		animator->Play("Play", true);
	}

	void Start() override {
		// Despawn exactly when the animation ends.
		const float len = m_clip.GetLengthSeconds();
		Object::Destroy(GetGameObject(), len > 0.0f ? len : 0.1f);
	}

	static std::string BuildVfxSheetKey(const std::string& path, const Vector2i& frame, bool useColorKey, const Vector3i& ck) {
		// Stable-ish key for caching. This still keeps the *cache* in AssetManager,
		// but avoids forcing scenes to preload or to pass keys around.
		std::string k = "sheet.vfx.";
		k += path;
		k += "|" + std::to_string(frame.x) + "x" + std::to_string(frame.y);
		if (useColorKey) {
			k += "|ck=" + std::to_string(ck.x) + "," + std::to_string(ck.y) + "," + std::to_string(ck.z);
		}
		return k;
	}

	void BuildAssetsIfNeeded() {
		// Build per-instance controller/clip (cheap), but load the sheet through AssetManager's cache.
		if (m_built) return;

		if (m_sheetPath.empty()) {
			THROW_ENGINE_EXCEPTION("OneShotVFXBehaviour has no spritesheet path configured");
		}
		if (m_frameSize.x <= 0 || m_frameSize.y <= 0) {
			THROW_ENGINE_EXCEPTION("OneShotVFXBehaviour has an invalid frame size");
		}
		if (m_fps <= 0.0f) {
			m_fps = 16.0f;
		}

		const std::string sheetKey = BuildVfxSheetKey(m_sheetPath, m_frameSize, m_useColorKey, m_colorKey);
		if (m_useColorKey) {
			m_sheet = LoadSpriteSheet(sheetKey, m_sheetPath, m_frameSize, m_colorKey);
		} else {
			m_sheet = LoadSpriteSheet(sheetKey, m_sheetPath, m_frameSize);
		}
		if (!m_sheet || !m_sheet->IsValid()) {
			THROW_ENGINE_EXCEPTION("Failed to load VFX spritesheet: " + m_sheetPath);
		}

		// Build a simple non-looping clip that plays every frame in the sheet.
		m_clip = AnimationClip{};
		m_clip.name = "Play";
		m_clip.sheet = m_sheet;
		m_clip.fps = m_fps;
		m_clip.loop = false;
		m_clip.frames.clear();

		const Vector2i texSize = m_sheet->texture->GetSize();
		const int cols = (m_frameSize.x > 0) ? (texSize.x / m_frameSize.x) : 0;
		const int rows = (m_frameSize.y > 0) ? (texSize.y / m_frameSize.y) : 0;
		const int total = std::max(0, cols * rows);
		for (int i = 0; i < total; ++i) {
			m_clip.frames.push_back(i);
		}

		// Build a controller with a single state.
		m_controller = AnimatorController{};
		AnimState st;
		st.id = 0;
		st.name = "Play";
		st.clip = &m_clip;
		m_controller.states = { st };
		m_controller.transitions.clear();
		m_controller.parameters.clear();
		m_controller.entryState = 0;

		m_built = true;
	}

	SpriteRenderer* sprite = nullptr;
	Animator* animator = nullptr;

	std::string m_sheetPath;
	Vector2i m_frameSize = Vector2i(16, 16);
	float m_fps = 16.0f;
	bool m_useColorKey = false;
	Vector3i m_colorKey = Vector3i(255, 0, 255);

	bool m_built = false;
	SpriteSheet* m_sheet = nullptr;
	AnimationClip m_clip{};
	AnimatorController m_controller{};
};

// Explosion VFX: a concrete one-shot VFX using explode16.bmp.
class ExplosionVFXBehaviour : public OneShotVFXBehaviour {
public:
	ExplosionVFXBehaviour() {
		SetComponentName("ExplosionVFXBehaviour");
		SetSpriteSheetPath("explode16.bmp");
		SetFrameSize(Vector2i(16, 16));
		SetFPS(24.0f);
		SetColorKey(Vector3i(255, 0, 255));
	}
};

class ExplosionVFX : public GameObject {
public:
	explicit ExplosionVFX(const std::string& name = "ExplosionVFX")
		: GameObject(name) {
		AddComponent<SpriteRenderer>();
		AddComponent<Animator>();
		AddComponent<ExplosionVFXBehaviour>();
	}
};

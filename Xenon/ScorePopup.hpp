#pragma once

#include "../GameEngine/GameEngine.h"
#include "XenonAssetKeys.h"

// -----------------------------------------------------------------------------
// ScorePopup / TextPopup
// Simple one-shot "text VFX" GameObjects:
// - shows text (e.g. "+100")
// - rises over ~1 second
// - then destroys itself
// -----------------------------------------------------------------------------


class ScorePopupBehaviour final : public MonoBehaviour {
public:
	explicit ScorePopupBehaviour(int scoreValue = 100)
		: MonoBehaviour(), m_scoreValue(scoreValue) {
	}

protected:
	void Awake() override {
		m_transform = GetTransform();
		m_text = GetComponent<TextRenderer>().get();
		if (!m_text) {
			THROW_ENGINE_EXCEPTION("ScorePopup is missing TextRenderer");
		}

		m_font = LoadBitmapFont(
			XenonAssetKeys::Fonts::Popup8x8,
			"Font8x8.bmp",
			Vector2i(8, 8),
			Vector3i(255, 0, 255)
		);

		m_text->SetFont(m_font);
		m_text->SetText("+" + std::to_string(m_scoreValue));
		m_text->SetAnchor(TextAnchor::Center);
		m_text->SetLayerOrder(100);
		m_text->SetExtraScale(1.0f);

		m_startPos = m_transform->GetPosition();
	}

	void Update() override {
		m_elapsed += DeltaTime();
		const float t = std::clamp(m_elapsed / m_duration, 0.0f, 1.0f);

		// Ease-out (fast start, slow finish)
		const float eased = 1.0f - (1.0f - t) * (1.0f - t);
		m_transform->SetPosition(m_startPos + Vector2f(0.0f, m_riseDistance * eased));

		if (m_elapsed >= m_duration) {
			Object::Destroy(GetGameObject());
		}
	}

private:
	int m_scoreValue = 0;
	float m_elapsed = 0.0f;
	float m_duration = 1.0f;
	float m_riseDistance = 28.0f;

	Transform* m_transform = nullptr;
	TextRenderer* m_text = nullptr;
	BitmapFont* m_font = nullptr;
	Vector2f m_startPos;
};

class ScorePopup final : public GameObject {
public:
	ScorePopup(const std::string& name, int scoreValue)
		: GameObject(name) {
		AddComponent<TextRenderer>();
		AddComponent<ScorePopupBehaviour>(scoreValue);
	}
};

class TextPopupBehaviour final : public MonoBehaviour {
public:
	explicit TextPopupBehaviour(std::string text = "")
		: MonoBehaviour(), m_textToDisplay(std::move(text)) {
	}

protected:
	void Awake() override {
		m_transform = GetTransform();
		m_text = GetComponent<TextRenderer>().get();
		if (!m_text) {
			THROW_ENGINE_EXCEPTION("TextPopup is missing TextRenderer");
		}

		m_font = LoadBitmapFont(
			XenonAssetKeys::Fonts::Popup8x8,
			"Font8x8.bmp",
			Vector2i(8, 8),
			Vector3i(255, 0, 255)
		);

		m_text->SetFont(m_font);
		m_text->SetText(m_textToDisplay);
		m_text->SetAnchor(TextAnchor::Center);
		m_text->SetLayerOrder(100);
		m_text->SetExtraScale(1.0f);

		m_startPos = m_transform->GetPosition();
	}

	void Update() override {
		m_elapsed += DeltaTime();
		const float t = std::clamp(m_elapsed / m_duration, 0.0f, 1.0f);

		const float eased = 1.0f - (1.0f - t) * (1.0f - t);
		m_transform->SetPosition(m_startPos + Vector2f(0.0f, m_riseDistance * eased));

		if (m_elapsed >= m_duration) {
			Object::Destroy(GetGameObject());
		}
	}

private:
	std::string m_textToDisplay;
	float m_elapsed = 0.0f;
	float m_duration = 1.0f;
	float m_riseDistance = 28.0f;

	Transform* m_transform = nullptr;
	TextRenderer* m_text = nullptr;
	BitmapFont* m_font = nullptr;
	Vector2f m_startPos;
};

class TextPopup final : public GameObject {
public:
	TextPopup(const std::string& name, const std::string& text)
		: GameObject(name) {
		AddComponent<TextRenderer>();
		AddComponent<TextPopupBehaviour>(text);
	}
};

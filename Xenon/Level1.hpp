#include <memory>

class Level1 : public Scene {
public:
	Level1()
		: Scene("Level1") {
	}

	void OnStart() override {
		auto& engine = SleeplessEngine::GetInstance();
		auto* renderer = engine.GetRenderer();
		auto* window = engine.GetWindow();
		if (!renderer || !window) {
			return;
		}

		if (!m_texture) {
			m_texture = std::make_unique<Texture>(
				*renderer,
				"PULife.bmp"
			);
		}

		auto spriteObject = CreateGameObject("PULife");
		auto sprite = spriteObject->AddComponent<SpriteRenderer>();
		sprite->SetTexture(m_texture.get());

		const Vector2i windowSize = window->GetSize();
		if (windowSize.x <= 0 || windowSize.y <= 0) {
			return;
		}

		const Vector2f center(static_cast<float>(windowSize.x) * 0.5f,
			static_cast<float>(windowSize.y) * 0.5f);
		const Vector2i spriteSize = m_texture->GetSize();
		const Vector2f halfSize(static_cast<float>(spriteSize.x) * 0.5f,
			static_cast<float>(spriteSize.y) * 0.5f);

		spriteObject->GetTransform()->SetPosition(center - halfSize);
	}

private:
	std::unique_ptr<Texture> m_texture;
};
#pragma once

#include <memory>
#include "Types.hpp"

namespace Engine {

	class GameObject;
	class Scene;

	class Component : public std::enable_shared_from_this<Component> {
	public:
		virtual ~Component() = default;

		// Lifecycle methods
		virtual void OnCreate() {}      // Called when component is added to GameObject
		virtual void OnDestroy() {}     // Called when component is being destroyed
		virtual void OnEnable() {}      // Called when GameObject becomes active
		virtual void OnDisable() {}     // Called when GameObject becomes inactive

		virtual void Update(float deltaTime) {}
		virtual void FixedUpdate(float fixedDeltaTime) {}
		virtual void LateUpdate(float deltaTime) {}

		virtual void Draw() {}          // For renderable components

		// Component state
		bool IsEnabled() const { return m_enabled; }
		void SetEnabled(bool enabled);

		// Getters
		GameObject* GetGameObject() const { return m_gameObject; }
		Scene* GetScene() const;

		// Component comparison
		template<typename T>
		bool IsType() const { return dynamic_cast<const T*>(this) != nullptr; }

	protected:
		GameObject* m_gameObject = nullptr;

	private:
		friend class GameObject;
		friend class Scene;

		bool m_enabled = true;
	};
}
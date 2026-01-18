#pragma once

#include "Component.h"
#include <functional>

namespace Engine {

	// Base class for all scriptable behaviours
	class Behaviour : public Component {
	public:
		virtual ~Behaviour() = default;

		// These can be overridden by user scripts
		virtual void Start() {}         // Called when GameObject starts (first Update)
		virtual void OnDestroy() override {} // Override Component's OnDestroy

	protected:
		// Helper methods for user scripts
		void Destroy(float delay = 0.0f);
		void Invoke(const std::function<void()>& method, float delay);
		void CancelInvoke();
	};
}
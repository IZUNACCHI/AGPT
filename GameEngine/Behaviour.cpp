#include "Behaviour.h"
#include "GameObject.h"
#include "Logger.h"

namespace Engine {

	void Behaviour::Destroy(float delay) {
		if (delay <= 0.0f) {
			if (m_gameObject) {
				m_gameObject->Destroy();
			}
		}
		else {
			// Note: For delayed destruction, you'd need a scheduler
			// This is a simplified implementation
			LOG_WARN("Delayed destruction not implemented yet");
			if (m_gameObject) {
				m_gameObject->Destroy();
			}
		}
	}

	void Behaviour::Invoke(const std::function<void()>& method, float delay) {
		// Note: This would require a scheduler system
		LOG_WARN("Invoke method not implemented yet");
	}

	void Behaviour::CancelInvoke() {
		// Note: This would require a scheduler system
		LOG_WARN("CancelInvoke not implemented yet");
	}
}
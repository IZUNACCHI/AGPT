#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "GameObject.h"

class Scene;

// Simple object pooling system for GameObjects.
class ObjectPool {
public:
	ObjectPool() = default;
	~ObjectPool() = default;

	// Acquire an object from the pool
	// If none exists, calls 'factory' to create a new one
	template<typename T, typename Factory>
	std::shared_ptr<T> Acquire(const std::string& poolKey, Factory&& factory) {
		// Look for available objects in the pool
		auto& bucket = m_pool[poolKey];
		// Check from the back (most recently released)
		while (!bucket.empty()) {
			auto& obj = bucket.back();
			bucket.pop_back();
			if (!obj) {
				continue;
			}
			// If it was destroyed, discard
			if (obj->IsDestroyed()) {
				continue;
			}
			// Reactivate and return if type matches
			auto casted = std::dynamic_pointer_cast<T>(obj);
			if (casted) {
				casted->SetActive(true);
				return casted;
			}
		}
		// None available then create
		auto created = factory();
		return created;
	}

	// Release an object back into the pool (deactivates it)
	void Release(const std::string& poolKey, const std::shared_ptr<GameObject>& obj);

	// Clears all pooled references
	void Clear();

private:
	std::unordered_map<std::string, std::vector<std::shared_ptr<GameObject>>> m_pool; // Pool buckets by key
};

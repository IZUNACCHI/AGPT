#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

/// Base class for all engine objects with identity and lifetime semantics.
class Object {
public:
	/// Creates an Object with the provided name.
	explicit Object(const std::string& name = "Object");
	/// Destroys the Object.
	virtual ~Object();

	/// Returns the unique instance ID for this Object.
	uint32_t GetInstanceID() const { return m_instanceID; }
	/// Returns the name of this Object.
	virtual const std::string& GetName() const { return m_name; }
	/// Sets the name of this Object.
	virtual void SetName(const std::string& name) { m_name = name; }

	/// Returns true if the object has been destroyed.
	bool IsDestroyed() const { return m_destroyed; }
	/// Returns true if the object is marked for destruction.
	bool IsMarkedForDestruction() const { return m_markedForDestruction; }

	/// Marks an object for destruction after the current update loop.
	static void Destroy(const std::shared_ptr<Object>& obj, float t = 0.0f);
	/// Marks an object for destruction after the current update loop.
	static void Destroy(Object* obj, float t = 0.0f);
	/// Instantiates (clones) an object and returns the clone.
	static std::shared_ptr<Object> Instantiate(const std::shared_ptr<Object>& obj);
	/// Finds all objects of a given type.
	static std::vector<std::shared_ptr<Object> > FindObjectsByType(const std::type_index& type, bool includeInactive);

	/// Finds all objects of a given type.
	template<typename T>
	static std::vector<std::shared_ptr<T> > FindObjectsByType(bool includeInactive);

	/// Returns false if the object is destroyed or invalid.
	explicit operator bool() const { return !m_destroyed; }

	/// Compares instance identity.
	friend bool operator==(const Object& lhs, const Object& rhs) {
		return lhs.m_instanceID == rhs.m_instanceID;
	}

	/// Compares instance identity.
	friend bool operator!=(const Object& lhs, const Object& rhs) {
		return lhs.m_instanceID != rhs.m_instanceID;
	}

	/// Processes queued destruction requests.
	static void ProcessDestroyQueue();

protected:
	/// Allows Scene to drive destruction.
	friend class Scene;
	/// Immediately destroys internal native resources.
	virtual void DestroyImmediateInternal();
	/// Marks this object as destroyed.
	void MarkDestroyed();

	/// Registers a new object in the global registry.
	static void RegisterObject(const std::shared_ptr<Object>& obj);
	/// Unregisters an object from the global registry.
	static void UnregisterObject(uint32_t instanceID);

private:
	/// Queues destruction of an object at a given time.
	static void QueueDestroy(const std::shared_ptr<Object>& obj, float executeAt);

	/// Unique instance ID.
	uint32_t m_instanceID = 0;
	/// Object name.
	std::string m_name;
	/// True once the object has been destroyed.
	bool m_destroyed = false;
	/// True once the object has been queued for destruction.
	bool m_markedForDestruction = false;
};

/// Finds all objects of a given type.
template<typename T>
std::vector<std::shared_ptr<T> > Object::FindObjectsByType(bool includeInactive) {
	std::vector<std::shared_ptr<T> > result;
	auto matches = FindObjectsByType(typeid(Object), includeInactive);
	result.reserve(matches.size());
	for (const auto& obj : matches) {
		if (auto casted = std::dynamic_pointer_cast<T>(obj)) {
			result.push_back(casted);
		}
	}
	return result;
}

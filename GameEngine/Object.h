#pragma once

#include <cstdint>
#include <string>

// Base class for all engine objects with a stable ID and name.
class Object {
public:
	explicit Object(std::string name = "");
	virtual ~Object() = default;

	uint64_t GetID() const { return m_id; }
	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	bool IsPendingDestroy() const { return m_pendingDestroy; }

protected:
	void MarkPendingDestroy() { m_pendingDestroy = true; }

private:
	static uint64_t s_nextId;
	uint64_t m_id = 0;
	std::string m_name;
	bool m_pendingDestroy = false;
};

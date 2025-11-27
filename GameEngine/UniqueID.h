#pragma once
#include <atomic>

namespace Engine
{
	inline uint64_t GenerateUniqueId()
	{
		static std::atomic<uint64_t> s_nextId{ 1 };
		return s_nextId++;
	}
}
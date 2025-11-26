#pragma once
#include <stdexcept>
#include <string>

class EngineException : public std::runtime_error {
public:
	explicit EngineException(const std::string& msg)
		: std::runtime_error(msg) {}
};


#pragma once
#include <string>
#include <string_view>

class Log {
public:
	static void Info(std::string_view msg);
	static void Warning(std::string_view msg);
	static void Error(std::string_view msg);   // throws EngineException on purpose
};
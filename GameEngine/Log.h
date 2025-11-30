#pragma once
#include <string_view>
#include <string>

enum class LogLevel {
	Info,
	Warning,
	Error,      // throws EngineException
	Debug       // only shows in debug builds or when enabled
};

class Log {
public:
	// The one function to rule them all - just like you wanted!
	static void log(LogLevel level, std::string_view message);

	// Nice shortcuts (optional but convenient)
	static void info(std::string_view msg) { log(LogLevel::Info, msg); }
	static void warning(std::string_view msg) { log(LogLevel::Warning, msg); }
	static void error(std::string_view msg) { log(LogLevel::Error, msg); }
	static void debug(std::string_view msg) { log(LogLevel::Debug, msg); }

	// Control debug logging
	static void SetDebugEnabled(bool enabled) { showDebug = enabled; }
	static bool IsDebugEnabled() { return showDebug; }

private:
	static bool showDebug;
};
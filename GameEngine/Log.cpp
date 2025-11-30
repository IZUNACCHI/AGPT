#include "Log.h"
#include "EngineException.h"
#include <iostream>
#include <chrono>
#include <iomanip>

bool Log::showDebug = true; // change to false in release if you want

static std::string CurrentTime() {
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&time), "%H:%M:%S");
	return ss.str();
}

void Log::log(LogLevel level, std::string_view message) {
	// Skip debug messages if disabled
	if (level == LogLevel::Debug && !showDebug) return;

	const char* prefix = "";
	const char* color = "\033[0m";  // reset

	switch (level) {
		case LogLevel::Info:    
			prefix = "[INFO]    "; 
			color = "\033[94m"; // light blue
			break; 
			case LogLevel::Warning:	
				prefix = "[WARNING] ";
				color = "\033[93m"; // yellow
				break; 
		case LogLevel::Error:   
			prefix = "[ERROR]   ";
			color = "\033[91m";  // red
			break;
		case LogLevel::Debug:   
			prefix = "[DEBUG]   ";
			color = "\033[96m"; // cyan
			break; 
	}

	// Print to console with color
	std::cout << color << "[" << CurrentTime() << "] " << prefix << message << "\033[0m" << std::endl;

	// Only Error throws
	if (level == LogLevel::Error) {
		throw EngineException(std::string(message));
	}
}
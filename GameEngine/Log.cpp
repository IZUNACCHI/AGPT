#include "Log.h"
#include "EngineException.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
static WORD originalColors = 0;
static bool colorsInit = false;
#endif

static std::string CurrentTime() {
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&time), "%H:%M:%S");
	return ss.str();
}

static void SetConsoleColor(int color) {
#if defined(_WIN32)
	if (!colorsInit) {
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(h, &info);
		originalColors = info.wAttributes;
		colorsInit = true;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#endif
}

static void ResetConsoleColor() {
#if defined(_WIN32)
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), originalColors);
#endif
}

void Log::Info(std::string_view msg) {
	std::cout << "\033[94m"; // light blue
	std::cout << "[" << CurrentTime() << "] ";
	std::cout << msg << "\033[0m" << std::endl;
}

void Log::Warning(std::string_view msg) {
	std::cout << "\033[93m"; // yellow
	std::cout << "[" << CurrentTime() << "] WARNING: " << msg << "\033[0m" << std::endl;
}

void Log::Error(std::string_view msg) {
	std::cout << "\033[91m"; // red
	std::cout << "[" << CurrentTime() << "] ERROR: " << msg << "\033[0m" << std::endl;
	throw EngineException(std::string(msg));
}
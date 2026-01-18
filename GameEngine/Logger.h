#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <mutex>
#include <windows.h>

enum class LogLevel {
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERR,
	FATAL
};

class Logger {
private:
	static Logger* instance;
	std::ofstream logFile;
	bool logToFile;
	std::mutex logMutex;

	Logger() : logToFile(false) {}
	~Logger() {
		if (logFile.is_open()) {
			logFile.close();
		}
	}

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	std::string getCurrentTime() {
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()) % 1000;

		std::tm tm;
		localtime_s(&tm, &time); // Use localtime_s for safety

		std::ostringstream oss;
		oss << std::put_time(&tm, "%H:%M:%S") << '.'
			<< std::setfill('0') << std::setw(3) << ms.count();
		return oss.str();
	}

	std::string getCurrentDate() {
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		std::tm tm;
		localtime_s(&tm, &time);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%d%m%Y");
		return oss.str();
	}

	std::string levelToString(LogLevel level) {
		switch (level) {
		case LogLevel::TRACE: return "TRACE";
		case LogLevel::DEBUG: return "DEBUG";
		case LogLevel::INFO:  return "INFO";
		case LogLevel::WARN:  return "WARN";
		case LogLevel::ERR:   return "ERROR";
		case LogLevel::FATAL: return "FATAL";
		default: return "UNKNOWN";
		}
	}

	void writeToFile(const std::string& message) {
		if (logToFile && logFile.is_open()) {
			logFile << message << std::endl;
			logFile.flush();
		}
	}

public:
	static Logger& getInstance() {
		static Logger instance;
		return instance;
	}

	void init(bool enableFileLogging = false) {
		std::lock_guard<std::mutex> lock(logMutex);
		logToFile = enableFileLogging;

		if (logToFile) {
			if (logFile.is_open()) {
				logFile.close();
			}

			std::string filename = "EngineLogs" + getCurrentDate() + ".log";
			logFile.open(filename, std::ios::out | std::ios::app);

			if (!logFile.is_open()) {
				std::cerr << "[ERROR] Failed to open log file: " << filename << std::endl;
				logToFile = false;
			}
			else {
				logFile << "=== Game Engine Log Started at " << getCurrentTime() << " ===" << std::endl;
			}
		}
	}

	void log(LogLevel level, const std::string& message, const char* file = "", int line = 0) {
		std::lock_guard<std::mutex> lock(logMutex);

		std::ostringstream oss;
		oss << "[" << getCurrentTime() << "] "
			<< "[" << levelToString(level) << "] ";

		if (file && file[0] != '\0') {
			oss << "[" << file;
			if (line > 0) {
				oss << ":" << line;
			}
			oss << "] ";
		}

		oss << message;

		// Set console color based on log level
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // Default white

		switch (level) {
		case LogLevel::TRACE: color = FOREGROUND_INTENSITY; break;
		case LogLevel::DEBUG: color = FOREGROUND_BLUE | FOREGROUND_GREEN; break;
		case LogLevel::INFO:  color = FOREGROUND_GREEN; break;
		case LogLevel::WARN:  color = FOREGROUND_RED | FOREGROUND_GREEN; break;
		case LogLevel::ERR:   color = FOREGROUND_RED; break;
		case LogLevel::FATAL: color = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
		}

		SetConsoleTextAttribute(hConsole, color);
		std::cout << oss.str() << std::endl;
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);


		writeToFile(oss.str());

		// For FATAL errors, you might want to add additional handling
		if (level == LogLevel::FATAL) {
			std::cerr << "FATAL ERROR OCCURRED! Application may terminate." << std::endl;
		}
	}

	// Convenience methods for each log level
	void trace(const std::string& message, const char* file = "", int line = 0) {
		log(LogLevel::TRACE, message, file, line);
	}

	void debug(const std::string& message, const char* file = "", int line = 0) {
		log(LogLevel::DEBUG, message, file, line);
	}

	void info(const std::string& message, const char* file = "", int line = 0) {
		log(LogLevel::INFO, message, file, line);
	}

	void warn(const std::string& message, const char* file = "", int line = 0) {
		log(LogLevel::WARN, message, file, line);
	}

	void error(const std::string& message, const char* file = "", int line = 0) {
		log(LogLevel::ERR, message, file, line);
	}

	void fatal(const std::string& message, const char* file = "", int line = 0) {
		log(LogLevel::FATAL, message, file, line);
	}

	bool isFileLoggingEnabled() const {
		return logToFile;
	}

	void setFileLogging(bool enable) {
		std::lock_guard<std::mutex> lock(logMutex);
		if (enable != logToFile) {
			init(enable);
		}
	}
};

// Macros for logging
#define LOG_TRACE(msg) Logger::getInstance().trace(msg, __FILE__, __LINE__)
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg)  Logger::getInstance().info(msg, __FILE__, __LINE__)
#define LOG_WARN(msg)  Logger::getInstance().warn(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance().error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg, __FILE__, __LINE__)

// General log macro (uses INFO level by default)
#define LOG(level, msg) Logger::getInstance().log(level, msg, __FILE__, __LINE__)
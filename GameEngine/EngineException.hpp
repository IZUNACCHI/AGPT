#pragma once

#include "Logger.h"
#include <string>
#include <exception>
#include <sstream>
#include <utility>

namespace Engine {

	class EngineException : public std::exception {
	public:
		// Constructor
		EngineException(std::string message, const char* file, int line) noexcept
			: m_message(std::move(message)), m_file(file), m_line(line) {
			LOG_ERROR(message);
		}

		// Copy constructor
		EngineException(const EngineException& other) noexcept = default;

		// Move constructor
		EngineException(EngineException&& other) noexcept = default;

		// Assignment operators
		EngineException& operator=(const EngineException& other) noexcept = default;
		EngineException& operator=(EngineException&& other) noexcept = default;

		~EngineException() noexcept override = default;

		// Get the error message
		const char* what() const noexcept override { return m_message.c_str(); }

		// Get the source file where the exception was thrown
		const char* getFile() const noexcept { return m_file; }

		// Get the line number where the exception was thrown
		int getLine() const noexcept { return m_line; }

		// Get the full formatted message including file and line
		std::string getFullMessage() const {
			std::ostringstream ss;
			ss << "Exception at " << m_file << ":" << m_line << " - " << m_message;
			return ss.str();
		}

		// Message building with operator<<
		template<typename T>
		EngineException& operator<<(const T& value) {
			std::ostringstream ss;
			ss << m_message << value;
			m_message = ss.str();
			return *this;
		}

		// Specialization for const char*
		EngineException& operator<<(const char* value) {
			m_message += value;
			return *this;
		}

		// Specialization for std::string
		EngineException& operator<<(const std::string& value) {
			m_message += value;
			return *this;
		}

	private:
		std::string m_message;
		const char* m_file;
		int m_line;
	};

} // namespace Engine

// Macros for throwing exceptions
// Basic throw macro
#define THROW_ENGINE_EXCEPTION(msg) \
    throw Engine::EngineException(msg, __FILE__, __LINE__)

// Conditional throw macro (throws if condition is true)
#define THROW_ENGINE_EXCEPTION_IF(condition, msg) \
    if (condition) throw Engine::EngineException(msg, __FILE__, __LINE__)

// Throw if pointer is null
#define THROW_IF_NULL(ptr, msg) \
    THROW_ENGINE_EXCEPTION_IF((ptr) == nullptr, msg)
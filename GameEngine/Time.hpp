#pragma once

#include <chrono>
#include <cstdint>
#include <algorithm>
#include <thread>


class Time {
public:
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;

	// Singleton
	static Time& Instance() {
		static Time instance;
		return instance;
	}

	// --- Core control ---
	void Initialize() {
		m_startTime = Clock::now();
		m_lastTime = m_startTime;
		m_now = m_startTime;

		m_deltaTime = 0.0;
		m_elapsedTime = 0.0;
		m_elapsedFixedTime = 0.0;
		m_accumulator = 0.0;

		m_frameCount = 0;
		m_fps = 0.0;
		m_fpsTimer = 0.0;
	}

	void Tick() {
		m_now = Clock::now();

		std::chrono::duration<float> frameDelta = m_now - m_lastTime;
		m_lastTime = m_now;

		m_deltaTime = frameDelta.count();

		// Clamp delta
		if (m_deltaTime > m_maxDeltaTime)
			m_deltaTime = m_maxDeltaTime;

		m_elapsedTime += m_deltaTime;
		m_accumulator += m_deltaTime;

		// FPS calculation
		m_frameCount++;
		m_fpsTimer += m_deltaTime;

		if (m_fpsTimer >= 1.0) {
			m_fps = static_cast<float>(m_frameCount) / m_fpsTimer;
			m_frameCount = 0;
			m_fpsTimer = 0.0;
		}
	}

	// --- Fixed timestep ---
	int CalculateFixedSteps() const {
		return static_cast<int>(m_accumulator / m_fixedDeltaTime);
	}

	void ConsumeFixedStep() {
		m_accumulator -= m_fixedDeltaTime;
		m_elapsedFixedTime += m_fixedDeltaTime;
	}

	// --- Getters ---
	float Now() const {
		return std::chrono::duration<float>(m_now - m_startTime).count();
	}

	float DeltaTime() const { return m_deltaTime; }
	float FixedDeltaTime() const { return m_fixedDeltaTime; }
	float ElapsedTime() const { return m_elapsedTime; }
	float ElapsedFixedTime() const { return m_elapsedFixedTime; }
	float Accumulator() const { return m_accumulator; }

	float FPS() const { return m_fps; }
	float TargetFPS() const { return m_targetFPS; }

	// --- Setters ---
	void SetFixedDeltaTime(float dt) { m_fixedDeltaTime = dt; }
	void SetMaxDeltaTime(float dt) { m_maxDeltaTime = dt; }

	void SetTargetFPS(float fps) {
		m_targetFPS = fps;
		m_targetFrameTime = (fps > 0.0f) ? (1.0f / fps) : 0.0f;
	}

	void WaitForTargetFPS() {
		if (m_targetFrameTime <= 0.0)
			return;

		auto frameEnd = Clock::now();
		std::chrono::duration<float> frameTime = frameEnd - m_lastTime;

		float remaining = m_targetFrameTime - frameTime.count();
		if (remaining > 0.0) {
			std::this_thread::sleep_for(
				std::chrono::duration<float>(remaining)
			);
		}
	}

	float TargetFrameTime() const { return m_targetFrameTime; }

private:
	Time() = default;

	// Time points
	TimePoint m_startTime{};
	TimePoint m_lastTime{};
	TimePoint m_now{};

	// Times (seconds)
	float m_deltaTime = 0.0f;
	float m_fixedDeltaTime = 1.0f / 60.0f;
	float m_maxDeltaTime = 0.25f;

	float m_elapsedTime = 0.0f;
	float m_elapsedFixedTime = 0.0f;
	float m_accumulator = 0.0f;

	// FPS
	float m_targetFPS = 60.0f;
	float m_targetFrameTime = 1.0f / 60.0f;

	float m_fps = 0.0f;
	float m_fpsTimer = 0.0f;
	uint32_t m_frameCount = 0;
};
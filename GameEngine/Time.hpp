#pragma once

#include <chrono>
#include <cstdint>
#include <algorithm>
#include <thread>
#include "Logger.h"

// Time
// - Unscaled time: real wall-clock time since engine start (always increases)
// - Scaled time: game time affected by timeScale (can pause/slow/reverse)
// - DeltaTime(): scaled delta (affected by timeScale)
// - UnscaledDeltaTime(): real delta (ignores timeScale)
// - Fixed steps only advance when scaled delta > 0
class Time {
public:
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;

	static Time& Instance() {
		static Time instance;
		return instance;
	}

	static void Initialize() {
		Time& instance = Instance();
		instance.m_startTime = Clock::now();
		instance.m_lastTime = instance.m_startTime;
		instance.m_now = instance.m_startTime;
		instance.m_frameStartTime = instance.m_startTime;

		instance.m_unscaledDeltaTime = 0.0f;
		instance.m_deltaTime = 0.0f;

		instance.m_unscaledElapsedTime = 0.0f;
		instance.m_elapsedTime = 0.0f;
		instance.m_elapsedFixedTime = 0.0f;
		instance.m_accumulator = 0.0f;

		instance.m_frameCount = 0;
		instance.m_fps = 0.0f;
		instance.m_fpsTimer = 0.0f;
		instance.m_showFPS = false;

		instance.m_timeScale = 1.0f;
	}

	static void Tick() {
		Time& instance = Instance();
		instance.m_frameStartTime = Clock::now();
		instance.m_now = instance.m_frameStartTime;

		std::chrono::duration<float> frameDelta = instance.m_frameStartTime - instance.m_lastTime;
		instance.m_lastTime = instance.m_frameStartTime;

		instance.m_unscaledDeltaTime = frameDelta.count();

		// Clamp unscaled delta to avoid huge jumps
		if (instance.m_unscaledDeltaTime > instance.m_maxDeltaTime)
			instance.m_unscaledDeltaTime = instance.m_maxDeltaTime;

		// Apply time scale
		instance.m_deltaTime = instance.m_unscaledDeltaTime * instance.m_timeScale;

		// Accumulate times
		instance.m_unscaledElapsedTime += instance.m_unscaledDeltaTime;
		instance.m_elapsedTime += instance.m_deltaTime; // scaled time (can go backwards)

		// Fixed step accumulator only moves forward when scaled delta is positive.
		if (instance.m_deltaTime > 0.0f) {
			instance.m_accumulator += instance.m_deltaTime;
		}

		// FPS calculation uses unscaled time.
		instance.m_frameCount++;
		instance.m_fpsTimer += instance.m_unscaledDeltaTime;

		if (instance.m_fpsTimer >= 1.0f) {
			if (instance.m_showFPS) {
				LOG_INFO("----- FPS Report -----");
				LOG_INFO("Frames: " + std::to_string(instance.m_frameCount));
				LOG_INFO("FPS Timer: " + std::to_string(instance.m_fpsTimer));
				LOG_INFO("FPS: " + std::to_string(instance.m_fps));
				LOG_INFO("----------------------");
			}
			instance.m_fps = static_cast<float>(instance.m_frameCount) / instance.m_fpsTimer;
			instance.m_frameCount = 0;
			instance.m_fpsTimer = 0.0f;
		}
	}

	// --- Fixed timestep ---
	static int CalculateFixedSteps() {
		const Time& instance = Instance();
		return static_cast<int>(instance.m_accumulator / instance.m_fixedDeltaTime);
	}

	static void ConsumeFixedStep() {
		Time& instance = Instance();
		instance.m_accumulator -= instance.m_fixedDeltaTime;
		instance.m_elapsedFixedTime += instance.m_fixedDeltaTime;
	}

	// --- Time getters ---
	// Scaled game time (affected by timeScale; can go backwards)
	static float Now() { return Instance().m_elapsedTime; }
	// Unscaled real time (always increases)
	static float UnscaledNow() { return Instance().m_unscaledElapsedTime; }

	static float DeltaTime() { return Instance().m_deltaTime; }
	static float UnscaledDeltaTime() { return Instance().m_unscaledDeltaTime; }

	static float FixedDeltaTime() { return Instance().m_fixedDeltaTime; }
	static float ElapsedTime() { return Instance().m_elapsedTime; }
	static float UnscaledElapsedTime() { return Instance().m_unscaledElapsedTime; }
	static float ElapsedFixedTime() { return Instance().m_elapsedFixedTime; }
	static float Accumulator() { return Instance().m_accumulator; }

	static float FPS() { return Instance().m_fps; }
	static float TargetFPS() { return Instance().m_targetFPS; }
	static float TargetFrameTime() { return Instance().m_targetFrameTime; }

	// --- Time control ---
	static void SetTimeScale(float scale) { Instance().m_timeScale = scale; }
	static float GetTimeScale() { return Instance().m_timeScale; }
	static bool IsPaused() { return Instance().m_timeScale == 0.0f; }

	// --- Setters ---
	static void SetFixedDeltaTime(float dt) { Instance().m_fixedDeltaTime = dt; }
	static void SetMaxDeltaTime(float dt) { Instance().m_maxDeltaTime = dt; }

	static void SetTargetFPS(float fps) {
		Time& instance = Instance();
		instance.m_targetFPS = fps;
		instance.m_targetFrameTime = (fps > 0.0f) ? (1.0f / fps) : 0.0f;
		return;
	}

	// --- Frame rate control ---
	static void WaitForTargetFPS() {
		Time& instance = Instance();
		if (instance.m_targetFrameTime <= 0.0f) return;

		const auto targetEnd =
			instance.m_frameStartTime + std::chrono::duration_cast<Clock::duration>(
				std::chrono::duration<float>(instance.m_targetFrameTime)
			);

		auto now = Clock::now();
		if (now >= targetEnd) {
			return;
		}

		constexpr auto sleepGuard = std::chrono::milliseconds(5);
		auto remaining = targetEnd - now;

		if (remaining > sleepGuard) {
			std::this_thread::sleep_for(remaining - sleepGuard);
		}

		while (Clock::now() < targetEnd) {
			std::this_thread::yield();
		}
	}

	static void ToggleShowFPS() {
		Time& instance = Instance();
		instance.m_showFPS = !instance.m_showFPS;
	}

private:
	Time() = default;

	TimePoint m_startTime{};
	TimePoint m_lastTime{};
	TimePoint m_now{};
	TimePoint m_frameStartTime{};

	float m_unscaledDeltaTime = 0.0f;
	float m_deltaTime = 0.0f;

	float m_fixedDeltaTime = 1.0f / 60.0f;
	float m_maxDeltaTime = 0.25f;

	float m_unscaledElapsedTime = 0.0f;
	float m_elapsedTime = 0.0f;
	float m_elapsedFixedTime = 0.0f;
	float m_accumulator = 0.0f;

	float m_timeScale = 1.0f;

	float m_targetFPS = 60.0f;
	float m_targetFrameTime = 1.0f / 60.0f;

	float m_fps = 0.0f;
	float m_fpsTimer = 0.0f;
	uint32_t m_frameCount = 0;

	bool m_showFPS = false;
};

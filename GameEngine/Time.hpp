#pragma once

#include <chrono>
#include <cstdint>
#include <algorithm>
#include <thread>
#include "Logger.h"


class Time {
public:
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;

	// Singleton
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

		instance.m_deltaTime = 0.0f;
		instance.m_elapsedTime = 0.0f;
		instance.m_elapsedFixedTime = 0.0f;
		instance.m_accumulator = 0.0f;

		instance.m_frameCount = 0;
		instance.m_fps = 0.0f;
		instance.m_fpsTimer = 0.0f;
		instance.m_showFPS = false;
	}

	static void Tick() {
		Time& instance = Instance();
		instance.m_frameStartTime = Clock::now();
		instance.m_now = instance.m_frameStartTime;

		std::chrono::duration<float> frameDelta = instance.m_frameStartTime - instance.m_lastTime;
		instance.m_lastTime = instance.m_frameStartTime;

		instance.m_deltaTime = frameDelta.count();

		// Clamp delta
		if (instance.m_deltaTime > instance.m_maxDeltaTime)
			instance.m_deltaTime = instance.m_maxDeltaTime;

		instance.m_elapsedTime += instance.m_deltaTime;
		instance.m_accumulator += instance.m_deltaTime;

		// FPS calculation
		instance.m_frameCount++;
		instance.m_fpsTimer += instance.m_deltaTime;

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

	// --- Getters ---
	static float Now() {
		const Time& instance = Instance();
		return std::chrono::duration<float>(instance.m_now - instance.m_startTime).count();
	}

	static float DeltaTime() { return Instance().m_deltaTime; }
	static float FixedDeltaTime() { return Instance().m_fixedDeltaTime; }
	static float ElapsedTime() { return Instance().m_elapsedTime; }
	static float ElapsedFixedTime() { return Instance().m_elapsedFixedTime; }
	static float Accumulator() { return Instance().m_accumulator; }

	static float FPS() { return Instance().m_fps; }
	static float TargetFPS() { return Instance().m_targetFPS; }

	// --- Setters ---
	static void SetFixedDeltaTime(float dt) { Instance().m_fixedDeltaTime = dt; }
	static void SetMaxDeltaTime(float dt) { Instance().m_maxDeltaTime = dt; }

	static void SetTargetFPS(float fps) {
		Time& instance = Instance();
		instance.m_targetFPS = fps;
		instance.m_targetFrameTime = (fps > 0.0f) ? (1.0f / fps) : 0.0f;
		return;
	}

	//--- Frame rate control ---
	static void WaitForTargetFPS() {
		Time& instance = Instance();
		// No target FPS set
		if (instance.m_targetFrameTime <= 0.0f) return;

		// Calculate target end time
		const auto targetEnd =
			instance.m_frameStartTime + std::chrono::duration_cast<Clock::duration>(
				std::chrono::duration<float>(instance.m_targetFrameTime)
			);

		auto now = Clock::now();
		// Already past target time
		if (now >= targetEnd) {
			return;
		}

		// Sleep guard duration, to avoid oversleeping by OS scheduler
		constexpr auto sleepGuard = std::chrono::milliseconds(5);

		// Time remaining
		auto remaining = targetEnd - now;

		if (remaining > sleepGuard) {
			std::this_thread::sleep_for(remaining - sleepGuard);
		}

		while (Clock::now() < targetEnd) {
			std::this_thread::yield();
		}
	}


	static float TargetFrameTime() { return Instance().m_targetFrameTime; }

	static void ToggleShowFPS() {
		Time& instance = Instance();
		instance.m_showFPS = !instance.m_showFPS;
	}

private:
	Time() = default;

	// Time points
	TimePoint m_startTime{};
	TimePoint m_lastTime{};
	TimePoint m_now{};
	TimePoint m_frameStartTime{};

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

	bool m_showFPS = false;
};
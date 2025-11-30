// Time.cpp
#include "Time.hpp"
#include <chrono>
#include <algorithm>
#include <array>
#include <thread>

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double>;

struct Time::Impl {

	// Timing
	// Start time
	Clock::time_point startTime = Clock::now();
	//time of last frame measured (end of stat period)
	Clock::time_point lastFrameTime = startTime;

	// Delta times
	// True delta
	double rawDelta = 0.0;
	// delta smoothed over several frames
	double smoothedDelta = 0.016; // start at ~60 FPS
	// Total time since start
	double totalTime = 0.0;
	// Frame count since start
	uint64_t frameCount = 0;

	// FPS tracking 64 frames (~1 second)
	static constexpr size_t FPS_SAMPLES = 64;
	std::array<double, FPS_SAMPLES> recentDeltas{};
	size_t fpsIndex = 0;

	// Fixed timestep, for physics and fixed update. Works by accumulating time, and checking if it exceeds fixedTimestep, taking a step
	float fixedTimestep = 0.0f;     // 0 = disabled
	double accumulator = 0.0;

	// Frame Cap
	float targetFPS = 0.0f;         // 0 = unlimited
	// Time at start of frame
	Clock::time_point frameStartTime{};

	// Update called once per frame
	void Update() {
		// Mark frame start time for frame limiting
		frameStartTime = Clock::now();

		// Calculate delta time
		auto now = Clock::now();
		Duration elapsed = now - lastFrameTime;
		lastFrameTime = now;

		rawDelta = elapsed.count();
		totalTime = Duration(now - startTime).count();
		++frameCount;

		// Smooth delta (simple exponential moving average), 
		smoothedDelta = smoothedDelta * 0.9 + rawDelta * 0.1;

		// Store for FPS calculation
		recentDeltas[fpsIndex] = rawDelta;
		fpsIndex = (fpsIndex + 1) % FPS_SAMPLES;

		// Fixed timestep
		if (fixedTimestep > 0.0f) {
			accumulator += rawDelta;
		}
	}

	// FPS calculations

	// Current FPS based on average of recent frames
	float CurrentFPS() const {
		double sum = 0.0;
		for (double d : recentDeltas) sum += d;
		double avg = sum / FPS_SAMPLES;
		return avg > 0.0 ? static_cast<float>(1.0 / avg) : 0.0f;
	}

	// Average FPS over the recorded samples
	float AverageFPS() const {
		size_t count = std::min(FPS_SAMPLES, static_cast<size_t>(frameCount));

		// if no samples yet, return 0
		if (count == 0) return 0.0f;
		double sum = 0.0;

		// sum and calculate average
		for (size_t i = 0; i < count; ++i) {
			sum += recentDeltas[(fpsIndex + i) % FPS_SAMPLES];
		}
		return static_cast<float>(count / sum);
	}

	// Min or Max FPS over the recorded samples
	float MinMaxFPS(bool min) const {
		double best = min ? 999.0 : 0.0;

		// find min or max delta
		for (double d : recentDeltas) {
			if (d <= 0.0) continue;
			if (min ? (d > best) : (d < best)) best = d;
		}
		return best > 0.0 ? static_cast<float>(1.0 / best) : 0.0f;
	}

	// Fixed timestep steps this frame
	int StepsThisFrame() const {
		if (fixedTimestep <= 0.0f) return 0;
		return static_cast<int>(accumulator / fixedTimestep);
	}

	// Wait to enforce target FPS, if done too quickly may result in buggy looking visuals
	void WaitIfNeeded() {
		// No limit
		if (targetFPS <= 0.0f) return;

		double targetFrameTime = 1.0 / targetFPS;
		Duration frameTime = Clock::now() - frameStartTime;
		double slept = frameTime.count();

		// Sleep if we finished early
		if (slept < targetFrameTime) {
			auto sleepDuration = std::chrono::duration<double>(targetFrameTime - slept);
			auto sleepUntil = frameStartTime + sleepDuration;
			while (Clock::now() < sleepUntil) {
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
		}
	}
};

std::unique_ptr<Time::Impl> Time::impl = nullptr;

void Time::Update() {
	if (!impl) impl = std::make_unique<Impl>();
	impl->Update();
}

float Time::DeltaTime() { return static_cast<float>(impl->smoothedDelta); }
double Time::DeltaTimeRaw() { return impl->rawDelta; }
float Time::TotalTime() { return static_cast<float>(impl->totalTime); }
uint64_t Time::FrameCount() { return impl->frameCount; }

float Time::FPS() { return impl->CurrentFPS(); }
float Time::AverageFPS() { return impl->AverageFPS(); }
float Time::MinFPS() { return impl->MinMaxFPS(true); }
float Time::MaxFPS() { return impl->MinMaxFPS(false); }

void Time::SetFixedTimestep(float dt) { impl->fixedTimestep = dt; }
float Time::FixedDeltaTime() { return impl->fixedTimestep; }
int Time::FixedStepsThisFrame() { return impl->StepsThisFrame(); }
void Time::ConsumeFixedSteps(int s) { impl->accumulator -= s * impl->fixedTimestep; }

void Time::SetTargetFPS(float fps) { impl->targetFPS = fps; }
float Time::TargetFPS() { return impl->targetFPS; }
void Time::WaitIfNeeded() { impl->WaitIfNeeded(); }
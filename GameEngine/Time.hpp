// Time.hpp
#pragma once
#include <cstdint>

class Time {
public:
	// Call exactly once per frame (right after Input::PollEvents)
	static void Update();

	// ------------------- Delta & Total Time -------------------
	static float  DeltaTime();        // seconds since last frame (smoothed)
	static double DeltaTimeRaw();     // raw, unsmoothed (for debugging)
	static float  TotalTime();        // seconds since game start

	// ------------------- Frame Count -------------------
	static uint64_t FrameCount();     // total frames rendered

	// ------------------- FPS -------------------
	static float  FPS();              // current FPS (smoothed)
	static float  AverageFPS();       // over last ~1 second
	static float  MinFPS();           // lowest FPS in last second
	static float  MaxFPS();           // highest FPS in last second

	// ------------------- Fixed Timestep (Physics) -------------------
	static void   SetFixedTimestep(float fixedDt); // 244 fps = 1/0.004096s, 144 fps = 1/0.006944s, 120 fps = 1/0.008333s, 90 fps = 1/0.011111s, 60 fps = 1/0.016666s, 30 fps = 1/0.033333s
	static float  FixedDeltaTime();
	static int    FixedStepsThisFrame(); // how many physics steps to run

	// Call after physics to consume steps
	static void   ConsumeFixedSteps(int steps = 1);

	// ------------------- Frame Limiter -------------------
	static void   SetTargetFPS(float fps);     // 0 = unlimited
	static float  TargetFPS();
	static void   WaitIfNeeded();              // call at end of frame to wait if needed

private:
	struct Impl;
	static std::unique_ptr<Impl> impl;
};
#include "Audio.h"

#include "Logger.h"

#include <SDL3/SDL.h>

// Internal audio state
namespace {
	SDL_AudioDeviceID g_device = 0;
	bool g_initialized = false;
	float g_masterGain = 1.0f;
}

// audio system
bool Audio::Initialize() {
	// Already initialized
	if (g_initialized) {
		return true;
	}

	// Init SDL audio subsystem
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		LOG_ERROR("Failed to initialize SDL audio subsystem: " + std::string(SDL_GetError()));
		return false;
	}

	// Open default playback device
	SDL_AudioSpec want{};
	SDL_zero(want);
	want.format = SDL_AUDIO_F32; // friendly format for conversion/mixing
	want.channels = 2; // stereo
	want.freq = 48000; // standard sample rate

	g_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &want);
	// Failed to open device
	if (g_device == 0) {
		LOG_ERROR("Failed to open default playback device: " + std::string(SDL_GetError()));
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	// In SDL3 an opened device won't output sound until streams are bound and fed,
	// but resuming explicitly keeps behavior consistent.
	SDL_ResumeAudioDevice(g_device);

	g_initialized = true;
	LOG_INFO("Audio initialized.");
	return true;
}

// Shutdown audio system
void Audio::Shutdown() {
	if (!g_initialized) return;

	// Close device
	if (g_device != 0) {
		SDL_CloseAudioDevice(g_device);
		g_device = 0;
	}

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	g_initialized = false;
	LOG_INFO("Audio shutdown.");
}

bool Audio::IsInitialized() {
	return g_initialized;
}

SDL_AudioDeviceID Audio::GetDevice() {
	return g_device;
}

void Audio::PauseDevice(bool pause) {
	if (!g_initialized || g_device == 0) return;
	if (pause) {
		SDL_PauseAudioDevice(g_device);
	}
	else {
		SDL_ResumeAudioDevice(g_device);
	}
}

// Master volume
void Audio::SetMasterGain(float gain) {
	g_masterGain = gain;
}

// Get master volume
float Audio::GetMasterGain() {
	return g_masterGain;
}

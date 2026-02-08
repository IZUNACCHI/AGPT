#pragma once

#include <SDL3/SDL.h>



namespace Audio {
	// Initializes SDL audio subsystem + opens default playback device
	bool Initialize();

	// Closes device and quits SDL audio subsystem
	void Shutdown();

	bool IsInitialized();
	SDL_AudioDeviceID GetDevice();

	// Device-level pause (affects all bound streams)
	void PauseDevice(bool pause);

	// Global master gain applied to all AudioSource gain values
	void SetMasterGain(float gain);
	float GetMasterGain();
}

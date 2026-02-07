#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>
#include <vector>

// A loaded piece of PCM audio data (currently WAV via SDL_LoadWAV).
// This is an engine asset (owned/cached by AssetManager).
struct AudioClip {
	std::string name;                   // cache key / debug name
	SDL_AudioSpec spec{};               // format of the PCM data
	std::vector<std::uint8_t> pcm;      // raw PCM bytes in 'spec' format
};

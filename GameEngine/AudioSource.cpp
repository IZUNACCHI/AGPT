#include "AudioSource.h"

#include "Audio.h"
#include "AudioClip.h"
#include "Logger.h"

#include <algorithm>

AudioSource::AudioSource()
	: MonoBehaviour() {
	SetName("AudioSource");
}

AudioSource::~AudioSource() {
	DestroyStream();
}

void AudioSource::SetClip(AudioClip* clip) {
	if (m_clip == clip) return;
	m_clip = clip;
	// stop current playback when changing clip.
	Stop();
}

void AudioSource::SetGain(float gain) {
	m_gain = std::max(0.0f, gain);
	if (m_stream) {
		SDL_SetAudioStreamGain(m_stream, m_gain * Audio::GetMasterGain());
	}
}

void AudioSource::SetPitch(float ratio) {
	m_pitch = std::max(0.01f, ratio);
	if (m_stream) {
		SDL_SetAudioStreamFrequencyRatio(m_stream, m_pitch);
	}
}

void AudioSource::Play() {
	if (!m_clip) return;

	// Lazily init audio
	if (!Audio::IsInitialized()) {
		Audio::Initialize();
		if (!Audio::IsInitialized()) return;
	}

	CreateStreamIfNeeded();
	if (!m_stream) return;

	SDL_ClearAudioStream(m_stream);
	// Queue the full clip once to start playback.
	QueueFullClip();
	if (!m_loop) {
		// Tell SDL we're done providing input so it can drain.
		SDL_FlushAudioStream(m_stream);
	}
}

void AudioSource::Play(AudioClip* clip, bool loop) {
	SetClip(clip);
	SetLoop(loop);
	Play();
}

void AudioSource::Stop() {
	DestroyStream();
}

void AudioSource::Update() {
	
	// Ensure stream exists if playing.
	if (!m_stream || !m_clip) return;

	// Looping: re-queue clip when input runs low.
	if (m_loop) {
		const int queued = SDL_GetAudioStreamQueued(m_stream);
		if (queued < 0) {
			DestroyStream();
			return;
		}

		const int clipBytes = (int)m_clip->pcm.size(); // full clip size in bytes
		const int threshold = std::max(4096, clipBytes / 2); // re-queue when below half or 4KB
		// Re-queue full clip if below threshold.
		if (queued < threshold) {
			QueueFullClip();
		}
		return;
	}

	// One-shot: destroy stream once fully drained.
	const int queued = SDL_GetAudioStreamQueued(m_stream);
	const int available = SDL_GetAudioStreamAvailable(m_stream);
	if (queued == 0 && available == 0) {
		DestroyStream();
	}
}

void AudioSource::OnDisable() {
	Stop();
}

void AudioSource::OnDestroy() {
	Stop();
}

void AudioSource::CreateStreamIfNeeded() {
	if (m_stream) return;
	if (!m_clip) return;
	if (!Audio::IsInitialized()) return;

	const SDL_AudioDeviceID dev = Audio::GetDevice();
	if (dev == 0) return;

    
	// Create and bind stream
	m_stream = SDL_CreateAudioStream(&m_clip->spec, NULL);
	if (!m_stream) {
		LOG_ERROR("Failed to create audio stream: " + std::string(SDL_GetError()));
		return;
	}
	// Bind to device
	if (!SDL_BindAudioStream(dev, m_stream)) {
		LOG_ERROR("Failed to bind audio stream: " + std::string(SDL_GetError()));
		SDL_DestroyAudioStream(m_stream);
		m_stream = nullptr;
		return;
	}

	SDL_SetAudioStreamGain(m_stream, m_gain * Audio::GetMasterGain());
	SDL_SetAudioStreamFrequencyRatio(m_stream, m_pitch);
}

void AudioSource::DestroyStream() {
	if (!m_stream) return;
	SDL_UnbindAudioStream(m_stream);
	SDL_DestroyAudioStream(m_stream);
	m_stream = nullptr;
}

void AudioSource::QueueFullClip() {
	if (!m_stream || !m_clip) return;
	if (m_clip->pcm.empty()) return;

	if (!SDL_PutAudioStreamData(m_stream, m_clip->pcm.data(), (int)m_clip->pcm.size())) {
		LOG_ERROR("SDL_PutAudioStreamData failed: " + std::string(SDL_GetError()));
		DestroyStream();
	}
}

std::shared_ptr<Component> AudioSource::Clone() const {
	auto c = std::make_shared<AudioSource>();
	c->m_clip = m_clip;
	c->m_loop = m_loop;
	c->m_gain = m_gain;
	c->m_pitch = m_pitch;
	// Runtime stream is not cloned.
	return c;
}

#pragma once

#include "MonoBehaviour.h"

#include <SDL3/SDL.h>

struct AudioClip;

// AudioSource component: plays back an AudioClip with adjustable gain/pitch/looping.
class AudioSource : public MonoBehaviour {
public:
	AudioSource();
	~AudioSource() override;

	void SetClip(AudioClip* clip);
	AudioClip* GetClip() const { return m_clip; }

	void SetLoop(bool loop) { m_loop = loop; }
	bool GetLoop() const { return m_loop; }

	void SetGain(float gain);
	float GetGain() const { return m_gain; }

	void SetPitch(float ratio);
	float GetPitch() const { return m_pitch; }

	void Play();
	void Play(AudioClip* clip, bool loop = false);
	void Stop();

	bool IsPlaying() const { return m_stream != nullptr; }

	std::shared_ptr<Component> Clone() const override;

protected:
	void Update() override;
	void OnDisable() override;
	void OnDestroy() override;

private:
	void CreateStreamIfNeeded(); // creates m_stream from m_clip
	void DestroyStream();
	void QueueFullClip();

private:
	AudioClip* m_clip = nullptr;
	SDL_AudioStream* m_stream = nullptr;
	bool m_loop = false;
	float m_gain = 1.0f;
	float m_pitch = 1.0f;
};

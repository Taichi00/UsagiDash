#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include <xaudio2.h>
#include <x3daudio.h>
#include <memory>

class Audio;
class AudioEngine;

class AudioSource : public Component
{
public:
	AudioSource(const std::shared_ptr<Audio>& audio);
	AudioSource(const std::shared_ptr<Audio>& audio, const float radius);
	~AudioSource();

	void Update(const float delta_time);

	void Play(const float volume = 1.0f, const bool loop = false);
	void Playing(const float volume = 1.0f, const bool loop = false);
	void Stop();

	void SetVolume(const float volume);
	void SetPitch(const float pitch);

private:
	void UpdateEmitterState();

private:
	AudioEngine* engine_ = nullptr;

	std::shared_ptr<Audio> audio_;
	IXAudio2SourceVoice* source_voice_ = nullptr;

	X3DAUDIO_EMITTER emitter_ = {};

	Vec3 prev_position_;

	bool is_3d_ = false;
	bool is_playing_ = false;
};
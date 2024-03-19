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
	AudioSource();
	AudioSource(
		std::shared_ptr<Audio> audio, 
		const float volume = 1,
		const float pitch = 1,
		const float radius = 0
	);
	~AudioSource();

	void Update(const float delta_time);

	// audio resource Çì«Ç›çûÇﬁ
	void Load(
		std::shared_ptr<Audio> audio,
		const float volume = 1,
		const float pitch = 1,
		const float radius = 0
	);

	// çƒê∂
	void Play(const bool loop = false);
	// åpë±çƒê∂
	void Playing(const bool loop = false);
	// í‚é~
	void Stop();

	void SetVolumePercentage(const float p);
	void SetPitchPercentage(const float p);

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

	// âπó 
	float volume_ = 1;
	// ÉsÉbÉ`
	float pitch_ = 1;

	bool is_3d_ = false;
	bool is_playing_ = false;
};
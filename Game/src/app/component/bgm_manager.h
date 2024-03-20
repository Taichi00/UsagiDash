#pragma once

#include "game/component/component.h"
#include <memory>

class AudioSource;
class Audio;

class BGMManager : public Component
{
public:
	BGMManager();
	~BGMManager() {}

	static BGMManager* Get()
	{
		return instance_;
	}

	bool Init() override;
	void Update(const float delta_time) override;

	void Play(std::shared_ptr<Audio> audio, const float volume = 1, const bool loop = true);

	void SetVolume(const float volume, const float time = 0);

private:
	static BGMManager* instance_;

	AudioSource* audio_source_ = nullptr;

	float current_volume_ = 1;
	float next_volume_ = 1;
	float volume_time_ = 0;
	float volume_timer_ = 0;
};
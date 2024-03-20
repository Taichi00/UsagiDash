#include "bgm_manager.h"
#include "game/entity.h"
#include "game/component/audio/audio_source.h"
#include "math/easing.h"

BGMManager* BGMManager::instance_ = nullptr;

BGMManager::BGMManager()
{
}

bool BGMManager::Init()
{
	instance_ = this;

	audio_source_ = GetEntity()->GetComponent<AudioSource>();

	return true;
}

void BGMManager::Update(const float delta_time)
{
	if (!audio_source_)
		return;

	if (volume_time_ > 0)
	{
		auto t = Easing::Linear(volume_timer_ / volume_time_);
		if (t >= 1)
		{
			t = 1;
			volume_time_ = 0;
		}

		auto volume = current_volume_ * (1 - t) + next_volume_ * t;
		audio_source_->SetVolumePercentage(volume);

		volume_timer_ += 60.0f * delta_time;
	}
}

void BGMManager::Play(std::shared_ptr<Audio> audio, const float volume, const bool loop)
{
	audio_source_->Load(audio, volume);
	audio_source_->Play(loop);
}

void BGMManager::SetVolume(const float volume, const float time)
{
	if (!audio_source_)
		return;

	if (time == 0)
	{
		audio_source_->SetVolumePercentage(volume);
	}
	else
	{
		current_volume_ = audio_source_->VolumePercentage();
		next_volume_ = volume;
		volume_time_ = time;
		volume_timer_ = 0;
	}
}

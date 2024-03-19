#include "timeline_player.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/audio/audio_source.h"

TimelinePlayer::TimelinePlayer(std::shared_ptr<Timeline> timeline)
{
	timeline_ = timeline;
}

bool TimelinePlayer::Init()
{
	GetTargetComponents();

	return true;
}

void TimelinePlayer::AfterUpdate(const float delta_time)
{
	if (!is_playing_)
		return;

	// �Đ��I���t���O
	bool end_flag = false;

	auto duration = timeline_->Duration();

	if (time_ > duration)
	{
		if (loop_)
		{
			// ���[�v�Đ�
			time_ = std::fmod(time_, duration);
			prev_time_ -= duration;
		}
		else
		{
			end_flag = true;
		}
	}

	// ���ׂẴg���b�N����������
	ProcessTracks();

	prev_time_ = time_;
	time_ += 60.0f * delta_time;

	// �Đ��I��
	if (end_flag)
	{
		is_playing_ = false;
	}
}

void TimelinePlayer::Play(const float speed, const bool loop)
{
	speed_ = speed;
	loop_ = loop;

	time_ = 0;
	prev_time_ = -0.1f;

	is_playing_ = true;

	// animator �̍Đ��L���[����ɂ���
	for (auto& pair : target_components_map_)
	{
		auto animator = pair.second.animator;
		if (animator)
		{
			animator->ClearQueue();
		}
	}

	// 1�t���[���ڂ��ɏ������Ă����i�J�����̍X�V���Ԃɍ���Ȃ����߁j
	ProcessTracks();
}

void TimelinePlayer::GetTargetComponents()
{
	for (const auto& track : timeline_->Tracks())
	{
		auto target = track.target;

		Components components = {};
		if (target_components_map_.contains(target))
		{
			components = target_components_map_[target];
		}

		switch (track.type)
		{
		case Timeline::TYPE_ANIMATION:
			components.animator = target->GetComponent<Animator>();
			break;
		case Timeline::TYPE_AUDIO:
			components.audio_source = target->GetComponent<AudioSource>();
			break;
		}

		target_components_map_[target] = components;
	}
}

void TimelinePlayer::ProcessTracks()
{
	for (const auto& track : timeline_->Tracks())
	{
		auto target = track.target;

		switch (track.type)
		{
		case Timeline::TYPE_ANIMATION:
			ProcessAnimationTracks(target, track.animation);
			break;
		case Timeline::TYPE_AUDIO:
			ProcessAudioTrack(target, track.audio);
			break;
		}
	}
}

void TimelinePlayer::ProcessAnimationTracks(Entity* target, const std::vector<Timeline::AnimationTrack>& tracks)
{
	auto animator = target_components_map_[target].animator;

	for (int i = 0; i < std::min((int)tracks.size(), ANIMATOR_LAYER_MAX); i++)
	{
		const auto& track = tracks[i];

		if (!track.animation_keys.empty())
		{
			auto key = GetCurrentKey<Timeline::AnimationKey>(track.animation_keys, time_, prev_time_);
			if (key.animation)
			{
				animator->Play(key.animation, key.speed, key.loop, 0.08f, i);
			}
		}
	}
}

void TimelinePlayer::ProcessAudioTrack(Entity* target, const Timeline::AudioTrack& track)
{
	auto audio_source = target_components_map_[target].audio_source;

	if (!track.audio_keys.empty())
	{
		auto key = GetCurrentKey<Timeline::AudioKey>(track.audio_keys, time_, prev_time_);
		if (key.audio)
		{
			audio_source->Load(key.audio, key.volume, key.pitch, key.radius);
			audio_source->Play(key.loop);
		}
	}

	if (!track.volume_keys.empty())
	{
		auto volume = Animation::GetCurrentFloat(track.volume_keys, time_);
		audio_source->SetVolumePercentage(volume);
	}

	if (!track.pitch_keys.empty())
	{
		auto pitch = Animation::GetCurrentFloat(track.pitch_keys, time_);
		audio_source->SetPitchPercentage(pitch);
	}
}

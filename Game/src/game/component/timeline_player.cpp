#include "timeline_player.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/audio/audio_source.h"
#include "game/component/camera.h"

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

	// 再生終了フラグ
	bool end_flag = false;

	auto duration = timeline_->Duration();

	if (time_ > duration)
	{
		if (loop_)
		{
			// ループ再生
			time_ = std::fmod(time_, duration);
			prev_time_ -= duration;
		}
		else
		{
			end_flag = true;
		}
	}

	// すべてのトラックを処理する
	ProcessTracks();

	prev_time_ = time_;
	time_ += 60.0f * speed_ * delta_time;

	// 再生終了
	if (end_flag)
	{
		is_playing_ = false;

		// 終了時関数の実行
		function_();
	}
}

void TimelinePlayer::Play(const float speed, const bool loop, const std::function<void()>& function)
{
	speed_ = speed;
	loop_ = loop;
	function_ = function;

	time_ = 0;
	prev_time_ = -0.1f;

	is_playing_ = true;

	// animator の再生キューを空にする
	for (auto& pair : target_components_map_)
	{
		auto animator = pair.second.animator;
		if (animator)
		{
			animator->ClearQueue();
		}
	}
}

float TimelinePlayer::Duration() const
{
	if (!timeline_)
		return 0.0f;

	return timeline_->Duration();
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
		case Timeline::TYPE_CAMERA:
			components.camera = target->GetComponent<Camera>();
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
		case Timeline::TYPE_CAMERA:
			ProcessCameraTrack(target, track.camera);
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
			Timeline::AnimationKey key = {};
			if (GetCurrentKey<Timeline::AnimationKey>(track.animation_keys, time_, prev_time_, key))
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
		Timeline::AudioKey key = {};
		if (GetCurrentKey<Timeline::AudioKey>(track.audio_keys, time_, prev_time_, key))
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

void TimelinePlayer::ProcessCameraTrack(Entity* target, const Timeline::CameraTrack& track)
{
	auto camera = target_components_map_[target].camera;

	if (!track.camera_keys.empty())
	{
		Timeline::CameraKey key = {};
		if (GetCurrentKey<Timeline::CameraKey>(track.camera_keys, time_, prev_time_, key))
		{
			if (key.focus_target)
			{
				camera->Focus(true);
				camera->SetFocusTarget(key.focus_target);
			}
			else
			{
				camera->Focus(false);
			}
		}
	}
}

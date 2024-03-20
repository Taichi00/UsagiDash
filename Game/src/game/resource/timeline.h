#pragma once

#include "game/resource/resource.h"
#include "game/resource/animation.h"
#include <vector>
#include <memory>

class Audio;
class Entity;

class Timeline : public Resource
{
public:
	struct AnimationKey : public Key
	{
		std::shared_ptr<Animation> animation = nullptr;
		float speed = 1;
		bool loop = false;
	};

	struct AudioKey : public Key
	{
		std::shared_ptr<Audio> audio = nullptr;
		bool loop = false;
		float volume = 1;
		float pitch = 1;
		float radius = 0;
	};

	struct CameraKey : public Key
	{
		Entity* focus_target = nullptr;
	};

	enum TrackType
	{
		TYPE_ANIMATION,
		TYPE_AUDIO,
		TYPE_CAMERA,
	};

	struct AnimationTrack
	{
		std::vector<AnimationKey> animation_keys;
	};

	struct AudioTrack
	{
		std::vector<AudioKey> audio_keys;
		std::vector<FloatKey> volume_keys;
		std::vector<FloatKey> pitch_keys;
	};

	struct CameraTrack
	{
		std::vector<CameraKey> camera_keys;
	};

	struct Track
	{
		TrackType type;
		Entity* target;

		std::vector<AnimationTrack> animation;
		AudioTrack audio;
		CameraTrack camera;
	};

	Timeline() {}
	~Timeline() {}

	// トラックを追加する
	void AddTrack(const Track& track);

	// トラックを取得する
	const std::vector<Track>& Tracks() const { return tracks_; }

	float Duration() const { return duration_; }
	void SetDuration(const float duration) { duration_ = duration; }

private:
	std::vector<Track> tracks_;

	// 継続時間
	float duration_ = 0;
};
#pragma once

#define NOMINMAX

#include "game/component/component.h"
#include "game/resource/timeline.h"
#include "game/resource/animation.h"
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

class Animator;
class AudioSource;
class Camera;

class TimelinePlayer : public Component
{
public:
	TimelinePlayer(std::shared_ptr<Timeline> timeline);
	~TimelinePlayer() {}

	bool Init() override;
	void AfterUpdate(const float delta_time) override;

	// タイムラインを再生する
	void Play(
		const float speed = 1,
		const bool loop = false,
		const std::function<void()>& function = []() {}
	);

	bool IsPlaying() const { return is_playing_; }

	void SetTime(const float time) { time_ = time; }

	float Duration() const;

private:
	void GetTargetComponents();

	void ProcessTracks();
	// アニメーショントラックの処理
	void ProcessAnimationTracks(Entity* target, const std::vector<Timeline::AnimationTrack>& tracks);
	// オーディオトラックの処理
	void ProcessAudioTrack(Entity* target, const Timeline::AudioTrack& track);
	// カメラトラックの処理
	void ProcessCameraTrack(Entity* target, const Timeline::CameraTrack& track);

	// キーを通過したときのみ返す
	template <typename T>
	bool GetCurrentKey(const std::vector<T>& keys, const float curr_time, const float prev_time, T& current_key)
	{
		int index1 = 0, index2 = 0;

		Animation::GetCurrentKeys(std::vector<Key>(keys.begin(), keys.end()), curr_time, index1, index2);

		auto& key1 = keys[index1];

		if (prev_time < key1.time && key1.time <= curr_time)
		{
			current_key = key1;
			return true;
		}
		return false;
	}

private:
	std::shared_ptr<Timeline> timeline_;

	struct Components
	{
		Animator* animator = nullptr;
		AudioSource* audio_source = nullptr;
		Camera* camera = nullptr;
	};

	// ターゲットエンティティのコンポーネントポインタへのマップ
	std::unordered_map<Entity*, Components> target_components_map_;

	// 再生速度
	float speed_ = 1;
	// ループ再生フラグ
	bool loop_ = false;

	// 現在の再生時間
	float time_ = 0;
	float prev_time_ = 0;

	// 再生終了時に実行する関数
	std::function<void()> function_;

	bool is_playing_ = false;
};
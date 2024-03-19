#pragma once

#include "game/component/component.h"
#include "math/quaternion.h"
#include "math/vec.h"
#include "math/color.h"
#include "game/resource/animation.h"
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#define ANIMATOR_LAYER_MAX 5

class Animation;
class Bone;
class MeshRenderer;
class Control;

class Animator : public Component
{
public:
	Animator();
	Animator(std::shared_ptr<Animation> animation);
	Animator(const std::vector<std::shared_ptr<Animation>>& animations);

	bool Init() override;
	void AfterUpdate(const float delta_time) override;

	// アニメーションを登録する
	void RegisterAnimation(std::shared_ptr<Animation> animation);
	void RegisterAnimations(const std::vector<std::shared_ptr<Animation>>& animations);

	// 再生
	void Play(
		std::shared_ptr<Animation> animation, 
		float speed = 1.0f, 
		bool loop = true, 
		float blend_time = 0.08f,
		unsigned int layer = 0
	);
	void Play(
		const std::string& name, 
		float speed = 1.0f, 
		bool loop = true, 
		float blend_time = 0.08f,
		unsigned int layer = 0
	);

	// 再生（同じアニメーションを再生中ならそのまま）
	void Playing(
		std::shared_ptr<Animation> animation, 
		float speed = 1.0f, 
		bool loop = true, 
		float blend_time = 0.08f,
		unsigned int layer = 0
	);
	void Playing(
		const std::string& name, 
		float speed = 1.0f, 
		bool loop = true, 
		float blend_time = 0.08f,
		unsigned int layer = 0
	);

	// 再生キューに追加
	void Push(
		std::shared_ptr<Animation> animation,
		float speed = 1.0f, 
		bool loop = true, 
		float blend_time = 0.08f,
		unsigned int layer = 0
	);void Push(
		const std::string& name, 
		float speed = 1.0f, 
		bool loop = true, 
		float blend_time = 0.08f,
		unsigned int layer = 0
	);

	// 再生キューを空にする
	void ClearQueue();

	// 停止
	void Stop();

	// 名前からアニメーションを取得する
	std::shared_ptr<Animation> GetAnimation(const std::string& name) const;

	// 再生速度を設定
	void SetSpeed(float speed) { current_animations_[0].speed = speed; }
	void SetSpeed(unsigned int layer, float speed) { current_animations_[layer].speed = speed; }

	float Speed() const { return current_animations_[0].speed; }
	float Speed(unsigned int layer) const { return current_animations_[layer].speed; }

	float CurrentTime(unsigned int layer = 0) const { return current_animations_[layer].current_time; }
	float PreviousTime(unsigned int layer = 0) const { return current_animations_[layer].previous_time; }

	Animation* CurrentAnimation(unsigned int layer = 0) const 
	{ 
		return current_animations_[layer].animation.get(); 
	}

private:
	struct AnimationData
	{
		std::shared_ptr<Animation> animation = nullptr;

		float current_time = 0;
		float previous_time = 0;
		// 再生速度
		float speed = 1;
		// ループするかどうか
		bool loop = true;
		// ブレンド率
		float blend_ratio = 0;
		// ブレンドにかかる時間
		float blend_time = 0;

		// 再生開始時点の transform
		Vec3 start_position;
		Quaternion start_rotation;
		Vec3 start_scale;
	};

	// アニメーションを更新する
	void UpdateAnimation(unsigned int layer, const float delta_time);

	void ProcessChannels(unsigned int layer, const float current_time);

	void AnimateBone(
		const Animation::BoneChannel& channel, 
		const float current_time, 
		const float blend_ratio
	);
	void AnimateTransform(
		const Animation::TransformChannel& channel, 
		const float current_time,
		const Vec3& start_position,
		const Quaternion& start_rotation,
		const Vec3& start_scale
		);
	void AnimateGUI(
		const Animation::GUIChannel& channel, 
		const float current_time
	);

private:

	// アニメーションの名前マップ
	std::map<std::string, std::shared_ptr<Animation>> animation_map_;
	// 現在のアニメーション
	//std::shared_ptr<Animation> current_animation_;
	AnimationData current_animations_[ANIMATOR_LAYER_MAX] = {};
	// 再生キュー
	std::queue<AnimationData> animation_queues_[ANIMATOR_LAYER_MAX] = {};

	// 時間
	//float current_time_ = 0;
	//float previous_time_ = 0;
	//// 再生速度
	//float speed_ = 1;
	//// ループするかどうか
	//bool loop_ = true;

	//// ブレンド
	//float blend_ratio_ = 0;
	//float blend_time_ = 0;

	MeshRenderer* mesh_renderer_ = nullptr;
	Control* control_ = nullptr;
};
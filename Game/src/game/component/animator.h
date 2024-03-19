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

	// �A�j���[�V������o�^����
	void RegisterAnimation(std::shared_ptr<Animation> animation);
	void RegisterAnimations(const std::vector<std::shared_ptr<Animation>>& animations);

	// �Đ�
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

	// �Đ��i�����A�j���[�V�������Đ����Ȃ炻�̂܂܁j
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

	// �Đ��L���[�ɒǉ�
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

	// �Đ��L���[����ɂ���
	void ClearQueue();

	// ��~
	void Stop();

	// ���O����A�j���[�V�������擾����
	std::shared_ptr<Animation> GetAnimation(const std::string& name) const;

	// �Đ����x��ݒ�
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
		// �Đ����x
		float speed = 1;
		// ���[�v���邩�ǂ���
		bool loop = true;
		// �u�����h��
		float blend_ratio = 0;
		// �u�����h�ɂ����鎞��
		float blend_time = 0;

		// �Đ��J�n���_�� transform
		Vec3 start_position;
		Quaternion start_rotation;
		Vec3 start_scale;
	};

	// �A�j���[�V�������X�V����
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

	// �A�j���[�V�����̖��O�}�b�v
	std::map<std::string, std::shared_ptr<Animation>> animation_map_;
	// ���݂̃A�j���[�V����
	//std::shared_ptr<Animation> current_animation_;
	AnimationData current_animations_[ANIMATOR_LAYER_MAX] = {};
	// �Đ��L���[
	std::queue<AnimationData> animation_queues_[ANIMATOR_LAYER_MAX] = {};

	// ����
	//float current_time_ = 0;
	//float previous_time_ = 0;
	//// �Đ����x
	//float speed_ = 1;
	//// ���[�v���邩�ǂ���
	//bool loop_ = true;

	//// �u�����h
	//float blend_ratio_ = 0;
	//float blend_time_ = 0;

	MeshRenderer* mesh_renderer_ = nullptr;
	Control* control_ = nullptr;
};
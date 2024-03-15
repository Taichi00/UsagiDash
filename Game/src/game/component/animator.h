#pragma once

#include "game/component/component.h"
#include "math/quaternion.h"
#include "math/vec.h"
#include "math/color.h"
#include "game/animation.h"
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

class Animation;
class Bone;
class MeshRenderer;
class Control;

class Animator : public Component
{
public:
	struct AnimationArgs
	{
		std::string name;
		float speed;
		bool loop;
		float blend_time;
	};

	Animator();
	Animator(const std::shared_ptr<Animation>& animation);
	Animator(const std::vector<std::shared_ptr<Animation>>& animations);

	bool Init() override;
	void AfterUpdate(const float delta_time) override;

	// �A�j���[�V������o�^����
	void RegisterAnimation(const std::shared_ptr<Animation>& animation);
	void RegisterAnimations(const std::vector<std::shared_ptr<Animation>>& animations);

	// �Đ�
	void Play(std::string name, float speed = 1.0f, bool loop = true, float blend_time = 0.08f);

	// �Đ��i�����A�j���[�V�������Đ����Ȃ炻�̂܂܁j
	void Playing(std::string name, float speed = 1.0f, bool loop = true, float blend_time = 0.08f);

	// �Đ��L���[�ɒǉ�
	void Push(std::string name, float speed = 1.0f, bool loop = true, float blend_time = 0.08f);

	// ��~
	void Stop();

	// �Đ����x��ݒ�
	void SetSpeed(float speed);
	float Speed() const { return speed_; }

	float CurrentTime() const { return current_time_; }

private:
	void Play(AnimationArgs anim);

	void AnimateBone(const Animation::Channel::BoneChannel& channel, const float current_time);
	void AnimateGUI(const Animation::Channel::GUIChannel& channel, const float current_time);

	Vec2 CalcCurrentVec2(const std::vector<Animation::Vec2Key>& keys, const float current_time);
	Vec3 CalcCurrentVec3(const std::vector<Animation::Vec3Key>& keys, const float current_time);
	Quaternion CalcCurrentQuat(const std::vector<Animation::QuatKey>& keys, const float current_time);
	Color CalcCurrentColor(const std::vector<Animation::ColorKey>& keys, const float current_time);
	float CalcCurrentFloat(const std::vector<Animation::FloatKey>& keys, const float current_time);

	// ���ݎ��Ԃ̑O��̃L�[���擾����
	void GetCurrentKeys(const std::vector<Animation::Key>& keys, const float current_time, int& index1, int& index2);

	float GetEasingTime(const Animation::Key& key1, const Animation::Key& key2, const float current_time);

private:
	// �A�j���[�V�����̖��O�}�b�v
	std::map<std::string, std::shared_ptr<Animation>> animation_map_;
	// ���݂̃A�j���[�V����
	std::shared_ptr<Animation> current_animation_;
	// �Đ��L���[
	std::queue<AnimationArgs> animation_queue_;

	// ����
	float current_time_ = 0;
	// �Đ����x
	float speed_ = 1;
	// ���[�v���邩�ǂ���
	bool loop_ = true;

	// �u�����h
	float blend_ratio_ = 0;
	float blend_time_ = 0;

	MeshRenderer* mesh_renderer_ = nullptr;
	Control* control_ = nullptr;
};
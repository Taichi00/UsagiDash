#pragma once

#include <vector>
#include <string>
#include "game/resource/resource.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/color.h"
#include "math/easing.h"

class Bone;
class Control;
class Animator;
class TimelinePlayer;

struct Key
{
	float time;
	Easing::Type easing;
};

struct FloatKey : public Key
{
	float value;
};

struct Vec2Key : public Key
{
	Vec2 value;
};

struct Vec3Key : public Key
{
	Vec3 value;
};

struct QuatKey : public Key
{
	Quaternion value;
};

struct ColorKey : public Key
{
	Color value;
};

class Animation : public Resource
{
public:
	friend Animator;
	friend TimelinePlayer;

	struct BoneChannel
	{
		std::string name; // �{�[���̖��O
		std::vector<Vec3Key> scale_keys;
		std::vector<QuatKey> rotation_keys;
		std::vector<Vec3Key> position_keys;
	};

	struct TransformChannel
	{
		std::vector<Vec3Key> scale_keys;
		std::vector<QuatKey> rotation_keys;
		std::vector<Vec3Key> position_keys;
	};

	struct GUIChannel
	{
		std::string name; // GUI�v�f�̖��O
		std::vector<Vec2Key> position_keys;
		std::vector<ColorKey> color_keys;
		std::vector<FloatKey> rotation_keys;
		std::vector<Vec2Key> scale_keys;
	};

	// channel �̎�ʁi�A�j���[�V�����̑Ώہj
	enum ChannelType
	{
		TYPE_BONE,
		TYPE_TRANSFORM,
		TYPE_GUI,
	};

	struct Channel
	{
		ChannelType type;
		BoneChannel bone;
		TransformChannel transform;
		GUIChannel gui;
	};

	Animation();
	Animation(std::string name);

	// �`�����l����ǉ�����
	void AddChannel(const Channel& channel);

	void SetName(const std::string& name) { name_ = name; }
	void SetTicksPerSecond(float ticks);
	void SetDuration(float duration) { duration_ = duration; }

	const std::vector<Channel>& Channels() const { return channels_; }
	std::string Name() const { return name_; }
	float TicksPerSecond() const { return ticks_per_second_; }
	float Duration() const { return duration_; }

private:
	// ���ݎ��Ԃ̑O��̃L�[���擾����
	static void GetCurrentKeys(const std::vector<Key>& keys, const float current_time, int& index1, int& index2);
	// 2�̃L�[����C�[�W���O����������̎��Ԃ��擾����
	static float GetEasingTime(const Key& key1, const Key& key2, const float current_time);

	static Vec2 GetCurrentVec2(const std::vector<Vec2Key>& keys, const float current_time);
	static Vec3 GetCurrentVec3(const std::vector<Vec3Key>& keys, const float current_time);
	static Quaternion GetCurrentQuat(const std::vector<QuatKey>& keys, const float current_time);
	static Color GetCurrentColor(const std::vector<ColorKey>& keys, const float current_time);
	static float GetCurrentFloat(const std::vector<FloatKey>& keys, const float current_time);

private:
	std::vector<Channel> channels_;

	// �A�j���[�V������
	std::string name_;
	// ��b�ɍX�V������
	float ticks_per_second_;
	// �A�j���[�V�����̌p������
	float duration_;

};
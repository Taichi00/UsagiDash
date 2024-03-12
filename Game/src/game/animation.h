#pragma once

#include <vector>
#include <string>
#include <memory>
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/color.h"
#include "math/easing.h"

class Bone;
class Control;

class Animation
{
public:
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

	// channel �̎�ʁi�A�j���[�V�����̑Ώہj
	enum ChannelType
	{
		TYPE_BONE,
		TYPE_GUI,
	};

	struct Channel
	{
		ChannelType type;

		struct BoneChannel
		{
			std::string name; // �{�[���̖��O
			std::vector<Vec3Key> scale_keys;
			std::vector<QuatKey> rotation_keys;
			std::vector<Vec3Key> position_keys;
		} bone;

		struct GUIChannel
		{
			std::string name;
			std::vector<Vec2Key> position_keys;
			std::vector<ColorKey> color_keys;
			std::vector<FloatKey> rotation_keys;
			std::vector<Vec2Key> scale_keys;
		} gui;
	};

	Animation();
	Animation(std::string name);

	// �`�����l����ǉ�����
	void AddChannel(const Channel& channel);

	void SetName(const std::string& name) { name_ = name; }
	void SetTicksPerSecond(float ticks) { ticks_per_second_ = ticks; }
	void SetDuration(float duration) { duration_ = duration; }

	const std::vector<Channel>& Channels() const { return channels_; }
	std::string Name() const { return name_; }
	float TicksPerSecond() const { return ticks_per_second_; }
	float Duration() const { return duration_; }

private:
	std::vector<Channel> channels_;

	// �A�j���[�V������
	std::string name_;
	// ��b�ɍX�V������
	float ticks_per_second_;
	// �A�j���[�V�����̌p������
	float duration_;

};
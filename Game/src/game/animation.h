#pragma once

#include <vector>
#include <string>
#include <memory>
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/color.h"

class Bone;
class Control;

class Animation
{
public:
	struct Key
	{
		float time;
		float (*easing)(const float); // イージング関数への関数ポインタ
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

	// channel の種別（アニメーションの対象）
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
			std::string name; // ボーンの名前
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

	// チャンネルを追加する
	void AddChannel(const Channel& channel);

	void SetTicksPerSecond(float ticks) { ticks_per_second_ = ticks; }
	void SetDuration(float duration) { duration_ = duration; }
	void SetName(const std::string& name) { name_ = name; }

	const std::vector<Channel>& Channels() const { return channels_; }
	float TicksPerSecond() const { return ticks_per_second_; }
	float Duration() const { return duration_; }
	std::string Name() const { return name_; }

private:
	std::vector<Channel> channels_;
	float ticks_per_second_;
	float duration_;
	std::string name_;
};
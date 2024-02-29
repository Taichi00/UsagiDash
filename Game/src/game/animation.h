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
	};

	struct BoneChannel : public Channel
	{
		std::string name; // ボーンの名前
		std::vector<Vec3Key> scaling_keys;
		std::vector<QuatKey> rotation_keys;
		std::vector<Vec3Key> position_keys;
	};

	struct GUIChannel : public Channel
	{
		std::vector<Vec2Key> position_keys;
		std::vector<ColorKey> color_keys;
		std::vector<FloatKey> rotation_keys;
	};

	Animation();
	Animation(std::string name);

	void AddChannel(Channel* channel);

	void SetTicksPerSecond(float ticks) { ticks_per_second_ = ticks; }
	void SetDuration(float duration) { duration_ = duration; }
	void SetName(const std::string& name) { name_ = name; }

	std::vector<Channel*> Channels();
	float TicksPerSecond() { return ticks_per_second_; }
	float Duration() { return duration_; }
	std::string Name() { return name_; }

private:
	std::vector<std::shared_ptr<Channel>> channels_;
	float ticks_per_second_;
	float duration_;
	std::string name_;
};
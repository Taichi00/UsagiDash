#include "game/resource/animation.h"

Animation::Animation()
{
	ticks_per_second_ = 0;
	duration_ = 0;
}

Animation::Animation(std::string name) : Animation()
{
	name_ = name;

	channels_.clear();
}

void Animation::AddChannel(const Channel& channel)
{
	channels_.push_back(channel);
}

void Animation::SetTicksPerSecond(float ticks)
{
	ticks_per_second_ = ticks == 0 ? 25 : ticks;
}

void Animation::GetCurrentKeys(const std::vector<Key>& keys, const float current_time, int& index1, int& index2)
{
	index1 = 0;
	index2 = (int)keys.size() - 1;

	for (auto i = 0u; i < keys.size(); i++)
	{
		auto& key = keys[i];
		if (current_time < key.time)
		{
			index2 = i;
			break;
		}
		index1 = i;
	}
}

float Animation::GetEasingTime(const Key& key1, const Key& key2, const float current_time)
{
	float duration = key2.time - key1.time;
	return duration > 0 ? Easing::GetFunction(key2.easing)((current_time - key1.time) / duration) : 0;
}

Vec2 Animation::GetCurrentVec2(const std::vector<Vec2Key>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Vec2::Lerp(key1.value, key2.value, t);
}

Vec3 Animation::GetCurrentVec3(const std::vector<Vec3Key>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Vec3::Lerp(key1.value, key2.value, t);
}

Quaternion Animation::GetCurrentQuat(const std::vector<QuatKey>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Quaternion::Slerp(key1.value, key2.value, t);
}

Color Animation::GetCurrentColor(const std::vector<ColorKey>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Color::Lerp(key1.value, key2.value, t);
}

float Animation::GetCurrentFloat(const std::vector<FloatKey>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return key1.value * (1 - t) + key2.value * t;
}


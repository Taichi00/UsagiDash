#pragma once

#include <vector>
#include <string>
#include "Vec.h"
#include "Quaternion.h"

struct VectorKey
{
	double Time;
	Vec3 Value;
};

struct QuatKey
{
	double Time;
	Quaternion Value;
};

struct Channel
{
	std::string Name;
	std::vector<VectorKey> ScalingKeys;
	std::vector<QuatKey> RotationKeys;
	std::vector<VectorKey> PositionKeys;
};

class Animation
{
public:
	Animation();
	Animation(std::string name);

	void AddChannel(Channel channel);
	void SetTicksPerSecond(double ticks);
	void SetDuration(double duration);
	void SetName(std::string name);

	std::vector<Channel> GetChannels();
	double GetTicksPerSecond();
	double GetDuration();
	std::string GetName();

private:
	std::vector<Channel> m_channels;
	double m_ticksPerSecond;
	double m_duration;
	std::string m_name;
};
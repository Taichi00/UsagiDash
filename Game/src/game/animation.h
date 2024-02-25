#pragma once

#include <vector>
#include <string>
#include "math/vec.h"
#include "math/quaternion.h"

struct VectorKey
{
	double time;
	Vec3 value;
};

struct QuatKey
{
	double time;
	Quaternion value;
};

struct Channel
{
	std::string name;
	std::vector<VectorKey> scaling_keys;
	std::vector<QuatKey> rotation_keys;
	std::vector<VectorKey> position_keys;
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
	std::vector<Channel> channels_;
	double ticks_per_second_;
	double duration_;
	std::string name_;
};
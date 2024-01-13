#include "game/animation.h"

Animation::Animation()
{
}

Animation::Animation(std::string name)
{
	name_ = name;

	channels_.clear();
	ticks_per_second_ = 0;
	duration_ = 0;
}

void Animation::AddChannel(Channel channel)
{
	channels_.push_back(channel);
}

void Animation::SetTicksPerSecond(double ticks)
{
	ticks_per_second_ = ticks;
}

void Animation::SetDuration(double duration)
{
	duration_ = duration;
}

void Animation::SetName(std::string name)
{
	name_ = name;
}

std::vector<Channel> Animation::GetChannels()
{
	return channels_;
}

double Animation::GetTicksPerSecond()
{
	return ticks_per_second_;
}

double Animation::GetDuration()
{
	return duration_;
}

std::string Animation::GetName()
{
	return name_;
}

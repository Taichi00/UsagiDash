#include "game/animation.h"

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


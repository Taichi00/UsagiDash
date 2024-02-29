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

void Animation::AddChannel(Channel* channel)
{
	channels_.push_back(std::unique_ptr<Channel>(channel));
}

std::vector<Animation::Channel*> Animation::Channels()
{
	std::vector<Channel*> channels;
	for (auto& channel : channels_)
	{
		channels.push_back(channel.get());
	}
	return channels;
}

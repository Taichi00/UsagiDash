#include "Animation.h"

Animation::Animation()
{
}

Animation::Animation(std::string name)
{
	m_name = name;

	m_channels.clear();
	m_ticksPerSecond = 0;
	m_duration = 0;
}

void Animation::AddChannel(Channel channel)
{
	m_channels.push_back(channel);
}

void Animation::SetTicksPerSecond(double ticks)
{
	m_ticksPerSecond = ticks;
}

void Animation::SetDuration(double duration)
{
	m_duration = duration;
}

void Animation::SetName(std::string name)
{
	m_name = name;
}

std::vector<Channel> Animation::GetChannels()
{
	return m_channels;
}

double Animation::GetTicksPerSecond()
{
	return m_ticksPerSecond;
}

double Animation::GetDuration()
{
	return m_duration;
}

std::string Animation::GetName()
{
	return m_name;
}

#pragma once

#include <vector>
#include <string>

class TagManager
{
public:
	TagManager();
	~TagManager() = default;

	void AddTag(const std::string& tag);

private:
	std::vector<std::string> tags_;
};
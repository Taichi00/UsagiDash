#include "tag_manager.h"

TagManager::TagManager()
{
	tags_.clear();
}

void TagManager::AddTag(const std::string& tag)
{
	tags_.push_back(tag);
}

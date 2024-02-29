#pragma once

#include "game/entity.h"
#include <string>

class Tutorial : public Entity
{
public:
	Tutorial(const std::string& text, const float radius);
	~Tutorial() {}
};
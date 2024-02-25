#pragma once

#include "game/entity.h"
#include <string>

class Coin : public Entity
{
public:
	Coin(const std::string& name);
	~Coin();
};
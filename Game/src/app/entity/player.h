#pragma once

#include "game/entity.h"
#include <string>

class Player : public Entity
{
public:
	Player(const std::string& name);
	~Player() {}
};
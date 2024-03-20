#pragma once

#include "game/entity.h"

class Ball : public Entity
{
public:
	Ball(const float radius, const float mass);
	~Ball() {}
};
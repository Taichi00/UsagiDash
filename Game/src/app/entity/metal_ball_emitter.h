#pragma once

#include "game/entity.h"

class MetalBallEmitter : public Entity
{
public:
	MetalBallEmitter(const float radius, const float width, const float spawn_rate);
	~MetalBallEmitter() {}
};
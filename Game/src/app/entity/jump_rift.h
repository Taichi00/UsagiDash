#pragma once

#include "game/entity.h"
#include "math/vec.h"

class JumpRift : public Entity
{
public:
	JumpRift(const Vec3& direction, const float distance);
	~JumpRift() {}
};
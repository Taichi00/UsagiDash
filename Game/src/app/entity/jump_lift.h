#pragma once

#include "game/entity.h"
#include "math/vec.h"

class JumpLift : public Entity
{
public:
	JumpLift(const Vec3& direction, const float distance);
	~JumpLift() {}
};
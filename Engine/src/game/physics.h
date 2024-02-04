#pragma once

#include "math/vec.h"
#include <string>
#include <vector>

class Collider;

struct RaycastHit
{
	Collider* collider;
	Vec3 point;
	Vec3 normal;
	float distance;
};

class Physics
{
public:
	static bool Raycast(const Vec3& origin, const Vec3& direction, const float& max_distance,
		RaycastHit& hit, const std::vector<std::string>& mask_tags = {});
};
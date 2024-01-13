#pragma once

#include "math/vec.h"
#include <vector>

class SphereCollider;
class CapsuleCollider;
class FloorCollider;
class MeshCollider;
class Collider;

class Ray
{
public:
	Ray(Vec3 origin, Vec3 direction, float distance);
	~Ray();

	bool Intersects(SphereCollider* sphere);
	bool Intersects(CapsuleCollider* sphere);
	bool Intersects(FloorCollider* floor);
	bool Intersects(MeshCollider* collider);

public:
	Vec3 origin;
	Vec3 direction;
	float distance;

	std::vector<Collider*> hit_colliders;
	std::vector<Vec3> hit_normals
		;
	std::vector<float> hit_depths;
};
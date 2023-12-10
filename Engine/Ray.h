#pragma once

#include "Vec.h"
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

	std::vector<Collider*> hitColliders;
	std::vector<Vec3> hitNormals;
	std::vector<float> hitDistances;
};
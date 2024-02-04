#pragma once

#include "math/vec.h"
#include "math/aabb.h"
#include "game/component/collider/collider.h"
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

	AABB GetAABB();

	void AddHit(Collider::HitInfo hit);
	const std::vector<Collider::HitInfo>& GetHits();

	const Collider::HitInfo& GetNearestHit();

public:
	Vec3 origin;
	Vec3 direction;
	float distance;

private:
	AABB aabb_;

	std::vector<Collider::HitInfo> hits_;
	Collider::HitInfo nearest_hit_;
};
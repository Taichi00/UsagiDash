#pragma once

#include "Collider.h"
#include "SharedStruct.h"

class SphereCollider;

struct MeshColliderProperty
{
	CollisionModel Model;
};

class MeshCollider : public Collider
{
public:
	MeshCollider(MeshColliderProperty prop);
	~MeshCollider();

	bool Init();

	bool Intersects(SphereCollider* sphere);

private:
	Vec3 ClosestPointOnLineSegment(Vec3 a, Vec3 b, Vec3 point);

public:
	CollisionModel model;
};
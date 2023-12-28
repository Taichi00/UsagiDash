#pragma once

#include "Collider.h"

class FloorCollider;
class MeshCollider;
class CapsuleCollider;
class Ray;

struct SphereColliderProperty
{
	float Radius;
};

class SphereCollider : public Collider
{
public:
	SphereCollider(SphereColliderProperty prop);
	~SphereCollider();

	bool Init();

	bool Intersects(SphereCollider* sphere);
	bool Intersects(CapsuleCollider* capsule);
	bool Intersects(FloorCollider* floor);
	bool Intersects(MeshCollider* collider);
	bool Intersects(Ray* ray);

public:
	float radius;
};
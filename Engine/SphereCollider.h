#pragma once

#include "Collider.h"

class FloorCollider;
class MeshCollider;

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
	bool Intersects(FloorCollider* floor);
	bool Intersects(MeshCollider* collider);

public:
	float radius;
};
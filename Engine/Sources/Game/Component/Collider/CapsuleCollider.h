#pragma once

#include "Collider.h"

class FloorCollider;
class MeshCollider;
class SphereCollider;

struct CapsuleColliderProperty
{
	float Radius;
	float Height;
};

class CapsuleCollider : public Collider
{
public:
	CapsuleCollider(CapsuleColliderProperty prop);
	~CapsuleCollider();

	bool Init();

	bool Intersects(SphereCollider* sphere);
	bool Intersects(CapsuleCollider* capsule);
	bool Intersects(FloorCollider* floor);
	bool Intersects(MeshCollider* collider);

public:
	float radius;
	float height;
};
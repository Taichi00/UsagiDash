#pragma once

#include "Collider.h"

class SphereCollider;

class FloorCollider : public Collider
{
public:
	FloorCollider();
	~FloorCollider();

	bool Init();

	bool Intersects(SphereCollider* sphere);
	bool Intersects(FloorCollider* floor);
};
#pragma once

#include "game/component/collider/collider.h"

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
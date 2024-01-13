#include "game/component/collider/floor_collider.h"
#include "game/component/collider/sphere_collider.h"

FloorCollider::FloorCollider()
{
}

FloorCollider::~FloorCollider()
{
}

bool FloorCollider::Init()
{
	Collider::Init();
	return true;
}

bool FloorCollider::Intersects(SphereCollider* sphere)
{
	return sphere->Intersects(this);
}

bool FloorCollider::Intersects(FloorCollider* floor)
{
	return false;
}

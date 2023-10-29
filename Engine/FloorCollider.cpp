#include "FloorCollider.h"
#include "SphereCollider.h"

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

#include "game/component/collider/ray.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/sphere_collider.h"

Ray::Ray(Vec3 origin, Vec3 direction, float distance)
{
	this->origin = origin;
	this->direction = direction.Normalized();
	this->distance = distance;
}

Ray::~Ray()
{
}

bool Ray::Intersects(SphereCollider* sphere)
{
	return sphere->Intersects(this);
}

bool Ray::Intersects(CapsuleCollider* sphere)
{
	return false;
}

bool Ray::Intersects(FloorCollider* floor)
{
	return false;
}

bool Ray::Intersects(MeshCollider* collider)
{
	return collider->Intersects(this);
}
